#include "Common.h"
#include "Script.h"
#include "VirtualMachine.h"

int main(int argc, char* args[]){
  VirtualMachine vm;
  vm.debugMode = true;

  if (argc == 1) {
    vm.repl();
  } else if (argc == 2) {
    vm.runFile(args[1]);
  } else {
    fprintf(stderr, "Usage: virtual_machine [path]\n");
    exit(64);
  }
}
