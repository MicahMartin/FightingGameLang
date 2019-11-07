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

static Obj* allocateObject(size_t size, ObjType type, Object::objList* objLinkedList) {
  Obj* object = (Obj*)Object::reallocate(NULL, 0, size);
  object->type = type;
  objLinkedList->push_front(object);
  return object;
}

static inline uint32_t hashString(const char* chars, int length){
  uint32_t hash = std::hash<std::string>()(std::string(chars, length));
  return hash;
}

static ObjString* allocateString(char* chars, int length, uint32_t hash, Object::objList* objLinkedList, Object::stringList* stringList) {
  ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING, objLinkedList);
  string->length = length;
  string->chars = chars;
  string->hash = hash;

  return string;
}

ObjString* Object::takeString(char* chars, int length, Object::objList* objLinkedList, Object::stringList* stringList) {
  uint32_t hash = hashString(chars, length);
  return allocateString(chars, length, hash, objLinkedList, stringList);
}

ObjString* Object::copyString(const char* chars, int length, Object::objList* objLinkedList, Object::stringList* stringList) {
  uint32_t hash = hashString(chars, length);
  char* heapChars = ALLOCATE(char, length + 1);
  memcpy(heapChars, chars, length);
  heapChars[length] = '\0';

  return allocateString(heapChars, length, hash, objLinkedList, stringList);
}
