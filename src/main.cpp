#include "Common.h"
#include "Script.h"

int main(int argc, char* args[]){
  Script mainScript;

  uint8_t symbolIndex = mainScript.writeSymbol(100);
  mainScript.writeByte(OP_CONSTANT, 1);
  mainScript.writeByte(symbolIndex, 1);
  mainScript.writeByte(OP_RETURN, 1);
  mainScript.disassembleScript("mainScript");
}
