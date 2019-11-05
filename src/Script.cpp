#include "Script.h"

Script::Script(){
  code.reserve(8);
}

Script::~Script(){}

void Script::writeByte(uint8_t byte, int lineNumber){
  code.push_back(byte);
  lines.push_back(lineNumber);
}

uint8_t Script::writeSymbol(Value value){
  symbols.push_back(value);
  return symbols.size() - 1;
}

/************************************** debugging *************************************/
// TODO - template based debuggre
int Script::symbolInstruction(const char* name, int offset) {
  // get operand (symbol index)
  uint8_t symbolIndex = code[++offset];
  printf("%-16s %4d '", name, symbolIndex);
  // print the symbol
  Value symbol = symbols[symbolIndex];
  ValueFn::printValue(symbol);
  printf("'\n");

  // skip the operand
  return ++offset;
}

int Script::simpleInstruction(const char* name, int offset) {
  printf("%s\n", name);
  return ++offset;
}

int Script::disassembleInstruction(int offset){
  printf("%04d ", offset);
  if (offset > 0 && lines[offset] == lines[offset - 1]) {
    printf("   | ");
  } else {
    printf("%4d ", lines[offset]);
  }

  uint8_t instruction = code[offset];
  switch (instruction) {
    case OP_CONSTANT:
      return symbolInstruction("OP_CONSTANT", offset);
      break;
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);
      break;
    default:
      printf("unknown op code! %d\n", instruction);
      return ++offset;
  }
}

void Script::disassembleScript(const char* name){
  printf("== %s ==\n", name);

  for (int offset = 0; offset < code.size();) {
    offset = disassembleInstruction(offset);
  }
}

