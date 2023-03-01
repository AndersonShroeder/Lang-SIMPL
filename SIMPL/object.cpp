#include "object.hh"

// Creates a ObjString object on the heap and inits fields
// static ObjString* makeString(const char* chars)
// {
//     ObjString* stringObj;
//     stringObj->str = string(chars);
//     return stringObj;
// }

// ObjString* copyString(const char* chars)
// {
//     ObjString* stringObj;
//     stringObj->str = string(chars);

//     ObjString* interned = vm.strings.tableFindString(stringObj);
//     if (interned != NULL) return interned;

//     return makeString(chars);
// }

// void printObject(Value value)
// {
//     switch (OBJ_TYPE(value))
//     {
//         case OBJ_STRING:
//             printf("%s", AS_CSTRING(value));
//             break;
//     }
// }