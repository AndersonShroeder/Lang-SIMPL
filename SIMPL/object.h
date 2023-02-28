#ifndef simpl_object_h
#define simpl_object_h

#include "common.h"
#include "value.h"
#include "vm.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value))->chars)

enum ObjType
{
    OBJ_STRING,
};

class Obj
{
public:
    ObjType type;
    struct Obj *next;
};

class ObjString : public Obj
{
public:
    int length;
    char *chars;
    uint32_t hash;
};

ObjString *takeString(char *chars, int length);

ObjString *copyString(const char *chars, int lenght);

void printObject(Value value);

static bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif