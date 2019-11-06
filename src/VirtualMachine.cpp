#include "VirtualMachine.h"
#include "Object.h"


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

void VirtualMachine::runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fputs("\n", stderr);

  size_t instruction = instructionPointer - scriptPointer->scriptStart();
  int line = scriptPointer->lines[instruction];
  fprintf(stderr, "[line %d] in script\n", line);

  stack.reset();
}

inline bool VirtualMachine::isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

void VirtualMachine::concatenate() {
  ObjString* b = AS_STRING(stack.pop());
  ObjString* a = AS_STRING(stack.pop());

  int length = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);
  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString* result = Object::takeString(chars, length, &noMemoryLeaks);
  stack.push(OBJ_VAL(result));
}

inline bool VirtualMachine::valuesEqual(Value valueA, Value valueB) {
  if (valueA.type != valueB.type) return false;

  switch (valueA.type) {
    case VAL_BOOL:   return AS_BOOL(valueA) == AS_BOOL(valueB);
    case VAL_NIL:    return true;
    case VAL_NUMBER: return AS_NUMBER(valueA) == AS_NUMBER(valueB);
    case VAL_OBJ: {
      ObjString* aString = AS_STRING(valueA);
      ObjString* bString = AS_STRING(valueB);
      return aString->length == bString->length && memcmp(aString->chars, bString->chars, aString->length) == 0;
    }
  }
}

inline ExecutionCode VirtualMachine::run(){
  // lets go fast bb
  #define READ_BYTE() (*instructionPointer++)
  #define READ_SYMBOL() (scriptPointer->symbols[READ_BYTE()])
  #define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(stack.peek(0)) || !IS_NUMBER(stack.peek(1))) { \
        runtimeError("Gotta use numbers for binary operands my G, strings coming soon"); \
        return EC_RUNTIME_ERROR; \
      } \
      long b = AS_NUMBER(stack.pop()); \
      long a = AS_NUMBER(stack.pop()); \
      printf("doing %ld op %ld\n", a, b); \
      stack.push(valueType(a op b)); \
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
      case OP_NIL: stack.push(NIL_VAL); break;
      case OP_TRUE: stack.push(BOOL_VAL(true)); break;
      case OP_FALSE: stack.push(BOOL_VAL(false)); break;
      case OP_EQUAL: {
        Value b = stack.pop();
        Value a = stack.pop();
        stack.push(BOOL_VAL(valuesEqual(a, b)));
        break;
      }
      case OP_GREATER:  BINARY_OP(BOOL_VAL, >); break;
      case OP_LESS:     BINARY_OP(BOOL_VAL, <); break;
      case OP_ADD: {
        if (IS_STRING(stack.peek(0)) && IS_STRING(stack.peek(1))) {
          concatenate();
        } else if (IS_NUMBER(stack.peek(0)) && IS_NUMBER(stack.peek(1))) {
          long b = AS_NUMBER(stack.pop());
          long a = AS_NUMBER(stack.pop());
          stack.push(NUMBER_VAL(a + b));
        } else {
          runtimeError("Operands must be two numbers or two strings.");
          return EC_RUNTIME_ERROR;
        }
        break;
      }
      case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -); break;
      case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *); break;
      case OP_DIVIDE:   BINARY_OP(NUMBER_VAL, /); break;
      case OP_NOT:
        stack.push(BOOL_VAL(isFalsey(stack.pop())));
        break;
      case OP_NEGATE: {
        if (!IS_NUMBER(stack.peek(0))) {
          runtimeError("Operand must be a number.");
          return EC_RUNTIME_ERROR;
        }

        stack.push(NUMBER_VAL(-AS_NUMBER(stack.pop())));
        break;
      }
      case OP_RETURN: {
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
  compiler.setRecordListPointer(&noMemoryLeaks);

  if (!compiler.compile(source, &script)) {
    return EC_COMPILE_ERROR;
  }
  scriptPointer = &script;
  instructionPointer = scriptPointer->scriptStart();

  ExecutionCode result = run();
  // TODO: account for constant table / string table
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
