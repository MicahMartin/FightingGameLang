#ifndef _Script_h
#define _Script_h

#include <vector>
#include <unordered_map>
#include "Common.h"
#include "Value.h"
#include "Scanner.h"

typedef enum {
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_DEFINE_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_GLOBAL,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NOT,
  OP_NEGATE,
  OP_PRINT,
  OP_JUMP_IF_FALSE,
  OP_JUMP,
  OP_LOOP,
  OP_RETURN
} OpCode;

typedef struct {
  Token name;
  int depth;
} Local;

class Script {
public:
  Script();
  ~Script();

  void writeByte(uint8_t byte, int lineNumber);
  uint8_t writeSymbol(Value value);

  // return a pointer to the start of the script
  uint8_t* scriptStart();
  // debugging
  // TODO: abstract to template based debugger
  int jumpInstruction(const char* name, int sign, int offset);
  int byteInstruction(const char* name, int offset);
  int symbolInstruction(const char* name, int offset);
  int simpleInstruction(const char* name, int offset);

  int disassembleInstruction(int offset);
  void disassembleScript(const char* name);

  std::vector<Value> symbols;
  std::unordered_map<std::string, Value> globals;
  std::vector<int> lines;

  // TODO: Stop being lazy and make accessor funcs
  std::vector<uint8_t> code;
  Local locals[256];
  int localCount = 0;
  int scopeDepth = 0;
private:
};

#endif /*  */
