#ifndef _VirtualMachine_h
#define _VirtualMachine_h

#include "Common.h"
#include "Script.h"

class VirtualMachine {
public:
  VirtualMachine();
  ~VirtualMachine();
private:
  Script* scriptPointer;
};

#endif
