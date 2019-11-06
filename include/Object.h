#ifndef _Object_h
#define _Object_h

#include "Common.h"
#include "Value.h"
#include <forward_list>

#define OBJ_TYPE(value)         (AS_OBJ(value)->type)
#define IS_STRING(value)        Object::isObjType(value, OBJ_STRING)
#define AS_STRING(value)        ((ObjString*)AS_OBJ(value))         
#define AS_CSTRING(value)       (((ObjString*)AS_OBJ(value))->chars)
#define ALLOCATE(type, count) \
    (type*)Object::reallocate(NULL, 0, sizeof(type) * (count))


typedef enum {
  OBJ_STRING,
} ObjType;

struct sObj {
  ObjType type;
};

struct sObjString {
  Obj obj;
  int length;
  char* chars;
  uint32_t hash;
};

namespace  Object {
  void* reallocate(void* previous, size_t oldSize, size_t newSize);
  void printObject(Value value);

  ObjString* takeString(char* chars, int length, std::forward_list<Obj*>* objLinkedList);
  ObjString* copyString(const char* chars, int length, std::forward_list<Obj*>* objLinkedList);

  static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
  }
}

#endif /* _Object_h */
