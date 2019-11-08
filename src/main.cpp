#include "Common.h"
#include "Script.h"
#include "VirtualMachine.h"


static char* readFile(const char* path) {
  FILE* file = fopen(path, "rb");

  fseek(file, 0L, SEEK_END);
  size_t size = ftell(file);
  rewind(file);

  char* buffer = new char[size + 1];
  size_t bytesRead = fread(buffer, sizeof(char), size, file);
  buffer[bytesRead] = '\0';

  fclose(file);
  return buffer;
}

int main(int argc, char* args[]){
  VirtualMachine vm;
  vm.debugMode = true;

  Script script; 
  const char* source = readFile(args[1]);
  vm.compiler.compile(source, &script);

  vm.execute(&script);
}
