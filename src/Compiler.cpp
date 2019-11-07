#include "Compiler.h"


Compiler::Compiler(){}
Compiler::~Compiler(){}

void Compiler::error(const char* message) {
  errorAt(&parser.previous, message);
}

void Compiler::errorAtCurrent(const char* message){
  errorAt(&parser.current, message);
}


void Compiler::errorAt(Token* token, const char* message){
  if (parser.panicMode) return;
  parser.panicMode = true;

  fprintf(stderr, "[line %d] Error", token->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    fprintf(stderr, " at '%.*s'", token->length, token->start);
  }

  fprintf(stderr, ": %s\n", message);
  parser.hadError = true;
}

void Compiler::synchronize(){
  parser.panicMode = false;

  while (parser.current.type != TOKEN_EOF) {
    if (parser.previous.type == TOKEN_SEMICOLON) return;

    switch (parser.current.type) {
      case TOKEN_FUNC:
      case TOKEN_VAR:
      case TOKEN_FOR:
      case TOKEN_IF:
      case TOKEN_WHILE:
      case TOKEN_PRINT:
      case TOKEN_RETURN:
      return;

      default: ; //nothing
    }

    advance();
  }
}

Script* Compiler::currentScript(){
  return scriptPointer;
}

void Compiler::printStatement(){
  expression();
  consume(TOKEN_SEMICOLON, "expected ; after value");
  emitByte(OP_PRINT);
}

void Compiler::parsePrecedence(Precedence precedence){
  advance();
  ParseFn prefixRule = getRule(parser.previous.type)->prefix;
  if (prefixRule == NULL) {
    error("Prefix rule is null");
    return;
  }
  bool canAssign = precedence <= PREC_ASSIGNMENT;
  // prefix rule should be Compiler::number
  (*this.*prefixRule)(canAssign);

  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    // theres gotta be a better way to write this lol
    (*this.*infixRule)(canAssign);
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    error("Invalid assignment target.");
    expression();
  }
}

ParseRule* Compiler::getRule(TokenType type) {
  return &rules[type];
}

uint8_t Compiler::makeConstant(Value value) {
  printf("writing symbol.. %s\n", value.as.string->c_str());
  uint8_t symbolIndex = scriptPointer->writeSymbol(value);
  return symbolIndex;
}

uint8_t Compiler::identifierConstant(Token* name) {
  std::string* stringVal = new std::string(name->start, name->length);
  Value val = STRING_VAL(stringVal);
  return makeConstant(val);
}

int Compiler::resolveLocal(Token* name) {
  for (int i = scriptPointer->localCount - 1; i >= 0; i--) {   
    Local* local = &scriptPointer->locals[i];                  
    if (identifiersEqual(name, &local->name)) {           
      if (local->depth == -1) {
        error("Cannot read local variable in its own initializer.");
      }
      return i;                                           
    }                                                     
  }
  return -1;                                              
}

bool Compiler::identifiersEqual(Token* a, Token* b) {
  if (a->length != b->length) return false;
  return memcmp(a->start, b->start, a->length) == 0;
}

void Compiler::addLocal(Token name) {
  if (scriptPointer->localCount == 256) {
    error("Too many local variables in function.");
    return;
  }
  Local* local = &scriptPointer->locals[scriptPointer->localCount++];
  local->name = name;
  local->depth = -1;
  local->depth = scriptPointer->scopeDepth;
}

void Compiler::emitConstant(Value value) {
  uint8_t symbolIndex = makeConstant(value);
  emitBytes(OP_CONSTANT, symbolIndex);
}

void Compiler::patchJump(int offset){
  // -2 to adjust for the bytecode for the jump offset itself.
  int jump = currentScript()->code.size() - offset - 2;

  if (jump > UINT16_MAX) {
    error("Too much code to jump over.");
  }

  currentScript()->code[offset] = (jump >> 8) & 0xff;
  currentScript()->code[offset + 1] = jump & 0xff;
}

int Compiler::emitJump(uint8_t offset) {
  emitByte(offset);
  emitByte(0xff);
  emitByte(0xff);
  return currentScript()->code.size()- 2;
}

void Compiler::number(bool canAssign) {
  long value = strtol(parser.previous.start, NULL, 10);
  emitConstant(NUMBER_VAL(value));
}

void Compiler::string(bool canAssign) {
  // TODO: keep a refernce to this somehow
  std::string* stringVal = new std::string(parser.previous.start+1, parser.previous.length-2);
  printf("creating string %s, the address %p\n", stringVal->c_str(), stringVal);
  Value val = STRING_VAL(stringVal);
  printf("creating string on some weird shit?? %s, the address %p\n", val.as.string->c_str(), val.as.string);
  emitConstant(val);
}

void Compiler::namedVariable(Token name, bool canAssign) {
  uint8_t getOp, setOp;
  int arg = resolveLocal(&name);
  if (arg != -1) {
    getOp = OP_GET_LOCAL;
    setOp = OP_SET_LOCAL;
  } else {
    arg = identifierConstant(&name);
    getOp = OP_GET_GLOBAL;
    setOp = OP_SET_GLOBAL;
  }

  if (canAssign && match(TOKEN_EQUAL)) {
    expression();
    emitBytes(setOp, (uint8_t)arg);
  } else {
    emitBytes(getOp, (uint8_t)arg);
  }
}

void Compiler::variable(bool canAssign) {
  namedVariable(parser.previous, canAssign);
}

void Compiler::unary(bool canAssign) {
  TokenType operatorType = parser.previous.type;

  // Compile the operand.
  parsePrecedence(PREC_UNARY);     

  // Emit the operator instruction.
  switch (operatorType) {
    case TOKEN_BANG: emitByte(OP_NOT); break;
    case TOKEN_MINUS: emitByte(OP_NEGATE); break;
    default:
      return; // Unreachable.
  }
}

void Compiler::binary(bool canAssign) {
  // Remember the operator.
  TokenType operatorType = parser.previous.type;

  // Compile the right operand.
  ParseRule* rule = getRule(operatorType);
  parsePrecedence((Precedence)(rule->precedence + 1));

  // Emit the operator instruction.
  switch (operatorType) {
    case TOKEN_BANG_EQUAL:    emitBytes(OP_EQUAL, OP_NOT); break;
    case TOKEN_EQUAL_EQUAL:   emitByte(OP_EQUAL); break;
    case TOKEN_GREATER:       emitByte(OP_GREATER); break;
    case TOKEN_GREATER_EQUAL: emitBytes(OP_LESS, OP_NOT); break;
    case TOKEN_LESS:          emitByte(OP_LESS); break;
    case TOKEN_LESS_EQUAL:    emitBytes(OP_GREATER, OP_NOT); break;
    case TOKEN_PLUS:          emitByte(OP_ADD); break;
    case TOKEN_MINUS:         emitByte(OP_SUBTRACT); break;
    case TOKEN_STAR:          emitByte(OP_MULTIPLY); break;
    case TOKEN_SLASH:         emitByte(OP_DIVIDE); break;
    default:
      return; // Unreachable.
  }
}

void Compiler::literal(bool canAssign) {
  switch (parser.previous.type) {
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    case TOKEN_NIL: emitByte(OP_NIL); break;
    case TOKEN_TRUE: emitByte(OP_TRUE); break;
    default:
      return; // Unreachable.
  }
}

void Compiler::grouping(bool canAssign) {
  expression();
  consume(TOKEN_RIGHT_PAREN, "you're missing a ')' after the expression dingus");
}


bool Compiler::check(TokenType expected) {
  return parser.current.type == expected;
}

bool Compiler::match(TokenType expected) {
  if(!check(expected)) return false;
  advance();
  return true;
}

void Compiler::advance() {
  parser.previous = parser.current;

  for (;;) {
    parser.current = scanner.scan();
    if (parser.current.type != TOKEN_ERROR) break;

    errorAtCurrent(parser.current.start);
  }
}

void Compiler::expression() {
  parsePrecedence(PREC_ASSIGNMENT);
}

void Compiler::expressionStatement() {
  expression();
  consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
  emitByte(OP_POP);
}

void Compiler::beginScope(){
  scriptPointer->scopeDepth++;
}

void Compiler::endScope(){
  scriptPointer->scopeDepth--;
  while (scriptPointer->localCount > 0 && scriptPointer->locals[scriptPointer->localCount - 1].depth > scriptPointer->scopeDepth) {
    emitByte(OP_POP);
    scriptPointer->localCount--;
  }
}

void Compiler::block(){
  while (!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
    declaration();
  }

  consume(TOKEN_RIGHT_BRACE, "Expect '}' after block.");
}

void Compiler::ifStatement() {
  consume(TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expect ')' after condition.");

  int thenJump = emitJump(OP_JUMP_IF_FALSE);
  statement();

  patchJump(thenJump);
}

void Compiler::statement() {
  if(match(TOKEN_PRINT)){
    printStatement();
  } else if (match(TOKEN_IF)) {        
    ifStatement(); 
  }
  else if (match(TOKEN_LEFT_BRACE)) {
    beginScope();                      
    block();                           
    endScope(); 
  }
  else {
    expressionStatement();
  }
}

void Compiler::declareVariable() {
  // Global variables are implicitly declared.
  if (scriptPointer->scopeDepth == 0) return;

  Token* name = &parser.previous;
  for (int i = scriptPointer->localCount - 1; i >= 0; i--) {
    Local* local = &scriptPointer->locals[i];
    if (local->depth != -1 && local->depth < scriptPointer->scopeDepth) {
      break;
    }

    if (identifiersEqual(name, &local->name)) {
      error("Variable with this name already declared in this scope.");
    }
  }
  addLocal(*name);
}

uint8_t Compiler::parseVariable(const char* errorMessage) {
  consume(TOKEN_IDENTIFIER, errorMessage);              
  declareVariable();
  if (scriptPointer->scopeDepth > 0) return 0;
  return identifierConstant(&parser.previous);
}

void Compiler::markInitialized() {
  scriptPointer->locals[scriptPointer->localCount - 1].depth = scriptPointer->scopeDepth;
}

void Compiler::defineVariable(uint8_t var) {
  if (scriptPointer->scopeDepth > 0) {
    markInitialized();
    return;
  }
  emitBytes(OP_DEFINE_GLOBAL, var);
}

void Compiler::varDeclaration() {
  uint8_t global = parseVariable("Expect variable name.");

  if (match(TOKEN_EQUAL)) {
    expression();
  } else {
    emitByte(OP_NIL);
  }
  consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

  defineVariable(global);
}

void Compiler::declaration() {
  if (match(TOKEN_VAR)) {
    varDeclaration();
  } else {
    statement();
  }
  if (parser.panicMode) synchronize();
}


void Compiler::consume(TokenType type, const char* syntaxErrorMessage){
  if (parser.current.type == type) {
    advance();
    return;
  }

  errorAtCurrent(syntaxErrorMessage);
}

bool Compiler::compile(const char *source, Script* script){
  scanner.initScanner(source);
  scriptPointer = script;

  parser.hadError = false;
  parser.panicMode = false;

  advance();
  while (!match(TOKEN_EOF)) {
    declaration();
  }
  emitByte(OP_RETURN);
  if (!parser.hadError) {
    script->disassembleScript("mainScript");
  }
  return !parser.hadError;
}

void Compiler::emitByte(uint8_t byte) {
  scriptPointer->writeByte(byte, parser.previous.line);
}

void Compiler::emitBytes(uint8_t firstByte, uint8_t secondByte) {
  emitByte(firstByte);
  emitByte(secondByte);
}

