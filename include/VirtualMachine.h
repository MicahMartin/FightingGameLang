#ifndef _VirtualMachine_h
#define _VirtualMachine_h

#include "Common.h"
#include "Script.h"
#include "Stack.h"
#include "Compiler.h"

typedef enum {
  EC_OK,
  EC_COMPILE_ERROR,
  EC_RUNTIME_ERROR,
} ExecutionCode;


class VirtualMachine {
public:
  VirtualMachine();
  ~VirtualMachine();

  void repl();
  void runFile(const char* path);
  ExecutionCode execute(const char* source);

  bool debugMode;

private:
  ExecutionCode run();
  void runtimeError(const char* format, ...);
  bool isFalsey(Value value);
  bool valuesEqual(Value valueA, Value valueB);
  void concatenate();

  Script* scriptPointer;
  uint8_t* instructionPointer;
  Compiler compiler;
  Stack stack;
};

#endif
