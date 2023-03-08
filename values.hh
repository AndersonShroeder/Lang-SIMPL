#ifndef simpl_value_h
#define simpl_value_h

#include "common.hh"

class Obj;

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

    std::variant<bool, double, std::shared_ptr<Obj>> val;
};

// Check if a Value type has a specific C type
#define IS_BOOL(value) ((value).type == VAL_BOOL)
#define IS_NIL(value) ((value).type == VAL_NIL)
#define IS_NUMBER(value) ((value).type == VAL_NUMBER)
#define IS_OBJ(value)   ((value).type == VAL_OBJ)

// converts Value types into C values
#define AS_OBJ(value)   ((std::get<std::shared_ptr<Obj>>(value.val)))
#define AS_BOOL(value) ((std::get<bool>(value.val)))
#define AS_NUMBER(value) ((std::get<double>(value.val)))

// converts C values into Value type -> allows dynamic typing
#define BOOL_VAL(value) ((Value){VAL_BOOL, value})
#define NIL_VAL ((Value){VAL_NIL, 0.0})
#define NUMBER_VAL(value) ((Value){VAL_NUMBER, value})
#define OBJ_VAL(object) ((Value){VAL_OBJ, object})


class ValueArray
{
public:
    std::vector<Value> values;

    ValueArray()
    {

    }

    void writeValue(Value value);

    int size();
};

void printValue(Value value);

bool valuesEqual(Value a, Value b);

#endif