#ifndef _Script_h
#define _Script_h

#include <vector>
#include "Common.h"
#include "Value.h"

typedef enum {
  OP_CONSTANT,
  OP_RETURN
} OpCode;

class Script {
public:
  Script();
  ~Script();

  void writeByte(uint8_t byte, int lineNumber);
  uint8_t writeSymbol(Value value);


  // debugging
  // TODO: abstract to template based debugger
  int symbolInstruction(const char* name, int offset);
  int simpleInstruction(const char* name, int offset);
  int disassembleInstruction(int offset);
  void disassembleScript(const char* name);
private:
  std::vector<uint8_t> code;
  std::vector<int> lines;
  std::vector<Value> symbols;
};

#endif /*  */
