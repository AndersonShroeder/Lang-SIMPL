#ifndef simpl_object_h
#define simpl_object_h

#include "common.hh"
#include "values.hh"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) isObjType(value, OBJ_STRING)

#define AS_STRING(value) (std::static_pointer_cast<ObjString>(AS_OBJ(value)))
#define AS_CSTRING(value) (std::static_pointer_cast<ObjString>(AS_OBJ(value)))

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


// Optimize this -> string view instead of string
class ObjString : public Obj
{
public:
    std::string str;
    
    bool operator==(const ObjString &other) const
    {
        return str == other.str;
    }
};

std::shared_ptr<ObjString> makeString(const char *chars, int length);

void printObject(Value value);

static bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#endif