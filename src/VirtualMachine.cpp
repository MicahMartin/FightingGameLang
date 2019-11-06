#include "VirtualMachine.h"


VirtualMachine::VirtualMachine(){}

VirtualMachine::~VirtualMachine(){}

void VirtualMachine::repl(){
  char line[1024];
  for (;;) {
    printf("> ");

    if (!fgets(line, sizeof(line), stdin)) {
      printf("\n");
      break;
    }

    execute(line);
  }
}

static char* readFile(const char* path);
void VirtualMachine::runFile(const char *path){
  char* source = readFile(path);
  ExecutionCode result = execute(source);
  // free(source);

  if (result == EC_COMPILE_ERROR) exit(65);
  if (result == EC_RUNTIME_ERROR) exit(70);
  delete source;
}

inline ExecutionCode VirtualMachine::run(){
  // lets go fast bb
  #define READ_BYTE() (*instructionPointer++)
  #define READ_SYMBOL() (scriptPointer->symbols[READ_BYTE()])
  #define BINARY_OP(op) \
    do { \
      long b = stack.pop(); \
      long a = stack.pop(); \
      printf("doing %ld op %ld\n", a, b); \
      stack.push(a op b); \
    } while (false)

  for (;;) {
    if (debugMode) { 
      printf("          ");
      for (Value* slot = stack.stack; slot < stack.stackTop; slot++) {
        printf("[ ");
        ValueFn::printValue(*slot);
        printf(" ]");
      }
      printf("\n");
      scriptPointer->disassembleInstruction((int)(instructionPointer - scriptPointer->scriptStart()));
    }

    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_CONSTANT: {
        Value symbol = READ_SYMBOL();
        stack.push(symbol);
        break;
      }
      case OP_ADD:      BINARY_OP(+); break;
      case OP_SUBTRACT: BINARY_OP(-); break;
      case OP_MULTIPLY: BINARY_OP(*); break;
      case OP_DIVIDE:   BINARY_OP(/); break;
      case OP_NEGATE: {
        Value symbol = READ_SYMBOL();
        stack.push(-stack.pop());
        break;
      }
      case OP_RETURN: {
        printf("returned! %ld\n", *(stack.stackTop-1));
        return EC_OK;
      }
    }
  }

  #undef READ_BYTE
  #undef READ_SYMBOL
  #undef BINARY_OP
}

ExecutionCode VirtualMachine::execute(const char* source){
  Script script;

  if (!compiler.compile(source, &script)) {
    return EC_COMPILE_ERROR;
  }
  scriptPointer = &script;
  instructionPointer = scriptPointer->scriptStart();

  ExecutionCode result = run();

  return EC_OK;
};

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
