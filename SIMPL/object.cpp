#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"

// Creates a ObjString object on the heap and inits fields
static ObjString* makeString(const char* chars)
{
    ObjString* stringObj;
    stringObj->str = string(chars)
    return stringObj;
}

ObjString* takeString(char* chars, int length)
{
    uint32_t hash = hashString(chars, length);

    ObjString* interned = vm.strings.tableFindString(chars, length, hash);
    if (interned != NULL)
    {
        FREE_ARRAY(char, chars, length + 1);
        return interned;
    }

    return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, int length)
{
    ObjString* stringObj;
    stringObj->str = string(chars);

    ObjString* interned = vm.strings.tableFindString(stringObj);
    if (interned != NULL) return interned;

    return makeString(chars);
}

void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}