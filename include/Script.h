#ifndef _Script_h
#define _Script_h

#include <vector>
#include "Common.h"
#include "Value.h"

typedef enum {
  OP_CONSTANT,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,
  OP_RETURN
} OpCode;

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
  int symbolInstruction(const char* name, int offset);
  int simpleInstruction(const char* name, int offset);
  int disassembleInstruction(int offset);
  void disassembleScript(const char* name);

  std::vector<Value> symbols;
private:
  std::vector<uint8_t> code;
  std::vector<int> lines;
};

#endif /*  */
