#include "Stack.h"

Stack::Stack(){
  stackTop = stack;
}

Stack::~Stack(){}

void Stack::reset(){
  stackTop = stack;
}

void Stack::push(Value value){
  *stackTop = value;
  stackTop++;
}

Value Stack::pop(){
  stackTop--;
  return *stackTop;
}

Value Stack::peek(int distance){
  return stackTop[-1 - distance];
}
