#ifndef simpl_value_h
#define simpl_value_h

#include "common.hh"

class Obj;
class ObjString;

enum ValueType
{
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ, // refers to heap allocated objects
};

// 16 byte values
struct Value
{
    ValueType type;
    union
    {
        bool boolean;
        double number;
        Obj* obj;
    } as;
};

// Check if a Value type has a specific C type
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value)   ((value).type == VAL_OBJ)

// converts Value types into C values
#define AS_OBJ(value)   ((value).as.obj)
#define AS_BOOL(value) ((value).as.boolean)
#define AS_NUMBER(value) ((value).as.number)

// converts C values into Value type -> allows dynamic typing
#define BOOL_VAL(value) ((Value){VAL_BOOL, {.boolean = value}})
#define NIL_VAL ((Value){VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, {.number = value}})
#define OBJ_VAL(object) ((Value){VAL_OBJ, {.obj = (Obj*)object}})


class ValueArray
{
public:
    std::vector<Value> values;

    ValueArray();

    void write_value_array(Value value);

    int size();
};

void printValue(Value value);

bool valuesEqual(Value a, Value b);

#endif