#ifndef _Compiler_h
#define _Compiler_h

#include "Common.h"
#include "Scanner.h"
#include "Script.h"

typedef struct {
  Token current;
  Token previous;
  bool hadError;
  bool panicMode;
} Parser;

typedef enum {
  PREC_NONE,
  PREC_ASSIGNMENT,  // =
  PREC_OR,          // or
  PREC_AND,         // and
  PREC_EQUALITY,    // == !=
  PREC_COMPARISON,  // < > <= >=
  PREC_TERM,        // + -
  PREC_FACTOR,      // * /
  PREC_UNARY,       // ! -
  PREC_CALL,        // () []
  PREC_PRIMARY
} Precedence;

class Compiler;
typedef void (Compiler::*ParseFn)();

typedef struct {
  ParseFn prefix;
  ParseFn infix;
  Precedence precedence;
} ParseRule;

class Compiler {
public:
  Compiler();
  ~Compiler();

  bool compile(const char* source, Script* script);
  void emitByte(uint8_t byte);
  void emitBytes(uint8_t firstByte, uint8_t secondByte);
  Script* currentScript();

private:
  Scanner scanner;
  Parser parser;
  Script* scriptPointer;

  void parsePrecedence(Precedence precedence);
  ParseRule* getRule(TokenType type);


  uint8_t makeConstant(Value value);
  void emitConstant(Value value);

  void number();
  void unary();
  void binary();
  void literal();
  void grouping();

  void advance();
  void expression();
  void consume(TokenType, const char* syntaxErrorMessage);

  void error(const char* message);
  void errorAtCurrent(const char* message);
  void errorAt(Token* token, const char* message);

  ParseRule rules[36] = {
    { &Compiler::grouping, NULL,    PREC_NONE },                  // TOKEN_LEFT_PAREN
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_RIGHT_PAREN
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_LEFT_BRACE
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_RIGHT_BRACE
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_COMMA
    { &Compiler::unary,    &Compiler::binary,  PREC_TERM },       // TOKEN_MINUS
    { NULL,     &Compiler::binary,  PREC_TERM },                  // TOKEN_PLUS
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_SEMICOLON
    { NULL,     &Compiler::binary,  PREC_FACTOR },                // TOKEN_SLASH
    { NULL,     &Compiler::binary,  PREC_FACTOR },                // TOKEN_STAR
    { &Compiler::unary,     NULL,    PREC_NONE },                             // TOKEN_BANG
    { NULL,     &Compiler::binary,    PREC_EQUALITY},                             // TOKEN_BANG_EQUAL
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_EQUAL
    { NULL,     &Compiler::binary,    PREC_EQUALITY },                             // TOKEN_EQUAL_EQUAL
    { NULL,     &Compiler::binary,    PREC_COMPARISON },                             // TOKEN_GREATER
    { NULL,     &Compiler::binary,    PREC_COMPARISON },                             // TOKEN_GREATER_EQUAL
    { NULL,     &Compiler::binary,    PREC_COMPARISON },                             // TOKEN_LESS
    { NULL,     &Compiler::binary,    PREC_COMPARISON },                             // TOKEN_LESS_EQUAL
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_IDENTIFIER
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_STRING
    { &Compiler::number,   NULL,    PREC_NONE },                  // TOKEN_NUMBER
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_AND
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_ELSE
    { &Compiler::literal,     NULL,    PREC_NONE },                             // TOKEN_FALSE
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_FOR
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_FUN
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_IF
    { &Compiler::literal,     NULL,    PREC_NONE },                             // TOKEN_NIL
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_OR
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_PRINT
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_RETURN
    { &Compiler::literal,     NULL,    PREC_NONE },                             // TOKEN_TRUE
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_VAR
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_WHILE
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_ERROR
    { NULL,     NULL,    PREC_NONE },                             // TOKEN_EOF
  };
};

#endif
