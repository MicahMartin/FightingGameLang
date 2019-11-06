#include "Stack.h"

Stack::Stack(){
  stackTop = stack;
}

Stack::~Stack(){}

void Stack::push(Value value){
  *stackTop = value;
  stackTop++;
}

Value Stack::pop(){
  stackTop--;
  return *stackTop;
}
