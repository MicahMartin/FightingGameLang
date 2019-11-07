#include "Compiler.h"
#include "Object.h"


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

  // prefix rule should be Compiler::number
  (*this.*prefixRule)();

  while (precedence <= getRule(parser.current.type)->precedence) {
    advance();
    ParseFn infixRule = getRule(parser.previous.type)->infix;
    // theres gotta be a better way to write this lol
    (*this.*infixRule)();
  }
}

ParseRule* Compiler::getRule(TokenType type) {
  return &rules[type];
}

uint8_t Compiler::makeConstant(Value value) {
  uint8_t symbolIndex = scriptPointer->writeSymbol(value);
  return symbolIndex;
}

uint8_t Compiler::identifierConstant(Token* name) {
  return makeConstant(OBJ_VAL(Object::copyString(name->start, name->length, recordListPointer, stringTablePointer)));
}

void Compiler::emitConstant(Value value) {
  uint8_t symbolIndex = makeConstant(value);
  emitBytes(OP_CONSTANT, symbolIndex);
}

void Compiler::number() {
  long value = strtol(parser.previous.start, NULL, 10);
  emitConstant(NUMBER_VAL(value));
}

void Compiler::string() {
  printf("stringTableSize before: %ld\n", stringTablePointer->size());
  emitConstant(OBJ_VAL(Object::copyString(parser.previous.start+1, parser.previous.length-2, recordListPointer, stringTablePointer)));
  printf("stringTableSize after: %ld\n", stringTablePointer->size());
}

void Compiler::namedVariable(Token name) {
  uint8_t arg = identifierConstant(&name);
  emitBytes(OP_GET_GLOBAL, arg);
}

void Compiler::variable() {
  namedVariable(parser.previous);
}

void Compiler::unary() {
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

void Compiler::binary() {
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

void Compiler::literal() {
  switch (parser.previous.type) {
    case TOKEN_FALSE: emitByte(OP_FALSE); break;
    case TOKEN_NIL: emitByte(OP_NIL); break;
    case TOKEN_TRUE: emitByte(OP_TRUE); break;
    default:
      return; // Unreachable.
  }
}

void Compiler::grouping() {
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

void Compiler::statement() {
  if(match(TOKEN_PRINT)){
    printStatement();
  } else {
    expressionStatement();
  }
}

uint8_t Compiler::parseVariable(const char* errorMessage) {
  consume(TOKEN_IDENTIFIER, errorMessage);              
  return identifierConstant(&parser.previous);
}

void Compiler::defineVariable(uint8_t var) {
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
    printf("good compile\n");
    // script->disassembleScript("mainScript");
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

void Compiler::setRecordListPointer(std::forward_list<Obj*>* pointer){
  recordListPointer = pointer;
}

void Compiler::setStringTablePointer(std::unordered_map<sObjString*, Value>* pointer){
  stringTablePointer = pointer;
}
