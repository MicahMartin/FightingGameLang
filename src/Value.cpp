#include "Value.h"

void ValueFn::printValue(Value value){
  printf("%ld", AS_NUMBER(value));
}
