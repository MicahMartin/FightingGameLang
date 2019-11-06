#ifndef _Stack_h
#define _Stack_h
#include "Common.h"
#include "Value.h"

class Stack {
public:
  Stack();
  ~Stack();

  void push(Value value);
  Value pop();
  
  Value stack[256];
  Value* stackTop;
};

#endif /* STACK_H */
