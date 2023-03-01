#ifndef simpl_object_h
#define simpl_object_h

#include "common.hh"
#include "values.hh"
#include "vm/vm.hh"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_STRING(value) ((ObjString *)AS_OBJ(value))
#define AS_CSTRING(value) (((ObjString *)AS_OBJ(value)))

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
    string str;
    
    bool operator==(const ObjString &other) const
    {
        return str == other.str;
    }
};



ObjString *takeString(char *chars);

ObjString *copyString(const char *chars);

void printObject(Value value);

static bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif