#include "object.hh"


std::shared_ptr<ObjString> makeString(const char* chars, int length)
{
    std::shared_ptr<ObjString> stringObj = std::make_shared<ObjString>();
    stringObj->str = std::string_view(chars).substr(0, length);

    // ObjString* interned = vm.strings.tableFindString(stringObj);
    // if (interned != NULL) return interned;

    return stringObj;
}

void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
            std::cout << AS_CSTRING(value)->str + '\0';
            break;
    }
}