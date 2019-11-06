#include "Object.h"
#include "Value.h"
#include "VirtualMachine.h"

#define ALLOCATE_OBJ(type, objectType, list) \
    (type*)allocateObject(sizeof(type), objectType, list)

void* Object::reallocate(void* previous, size_t oldSize, size_t newSize) {
  if (newSize == 0) {
    free(previous);
    return NULL;
  }

  return realloc(previous, newSize);
}

void Object::printObject(Value value) {
  switch (OBJ_TYPE(value)) {
    case OBJ_STRING:
      printf("%s", AS_CSTRING(value));
      break;
  }
}

static Obj* allocateObject(size_t size, ObjType type, std::forward_list<Obj*> objLinkedList) {
  Obj* object = (Obj*)Object::reallocate(NULL, 0, size);
  object->type = type;
  objLinkedList.push_front(object);
  return object;
}

static ObjString* allocateString(char* chars, int length, std::forward_list<Obj*> objLinkedList) {
  ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING, objLinkedList);
  string->length = length;
  string->chars = chars;

  return string;
}

ObjString* Object::takeString(char* chars, int length, std::forward_list<Obj*> objLinkedList) {
  return allocateString(chars, length, objLinkedList);
}

ObjString* Object::copyString(const char* chars, int length, std::forward_list<Obj*> objLinkedList) {
  char* heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';

  return allocateString(heapChars, length, objLinkedList);
}
