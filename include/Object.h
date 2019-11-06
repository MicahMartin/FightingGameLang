#ifndef _Object_h
#define _Object_h

#include "Common.h"
#include "Value.h"
#include <forward_list>
#include <functional>
#include <unordered_map>
#include <string>

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
  bool operator==(const sObjString& other) const {
    printf("helllooo?\n");
    return other.length == length && memcmp(other.chars, chars, other.length) == 0;
  }
};

namespace std {
  template <> struct hash<sObjString> {
    size_t operator()(const sObjString& key) const {
      printf("hash of the string: %ld\n", hash<string>()(string(key.chars)));
      return hash<string>()(string(key.chars, key.length));
    }
  };
}

namespace  Object {
  void* reallocate(void* previous, size_t oldSize, size_t newSize);
  void printObject(Value value);

  typedef std::forward_list<Obj*> objList;
  typedef std::unordered_map<sObjString*, Value> stringList;

  ObjString* takeString(char* chars, int length, objList* objLinkedList, stringList* stringHashTable);
  ObjString* copyString(const char* chars, int length, objList* objLinkedList, stringList* stringHashTable);

  static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
  }
}

#endif /* _Object_h */
