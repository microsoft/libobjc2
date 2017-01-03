#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>

#ifndef DEBUG_EXCEPTIONS
#define DEBUG_LOG(...)
#else
#define DEBUG_LOG(str, ...) fprintf(stderr, str, ## __VA_ARGS__)
#endif

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif
#if !__has_builtin(__builtin_unreachable)
#define __builtin_unreachable abort
#endif

using id = void*;
using Class = id;
using SEL = char*;
using IMP = id(*)(id, SEL);

#define nil ((id)nullptr)

extern "C" SEL sel_registerName(const char*);
extern "C" bool class_respondsToSelector(Class, SEL);
extern "C" IMP objc_msg_lookup(id, SEL);
extern "C" id objc_msgSend(id, SEL, ...);
extern "C" Class object_getClass(id);
extern "C" Class class_getSuperclass(Class);
extern "C" const char* class_getName(Class);

struct __ObjC_CatchableType {
    unsigned int flags;
    _TypeDescriptor* type;
    int mdisp;
    int pdisp;
    int vdisp;
    int size;
    void* copyFunction;
};

struct __ObjC_CatchableTypeArray {
    int count;
    __ObjC_CatchableType* types[0];
};

namespace {

    std::string mangleClassName(Class cls) {
        // TODO - this name mangling correctly:
        return std::string(".PAAU") + class_getName(cls) + "@@";
    }

    void fillCatchableType(__ObjC_CatchableType* exceptType) {
        exceptType->flags = 1;
        exceptType->mdisp = 0;
        exceptType->pdisp = -1;
        exceptType->vdisp = 0;
        exceptType->size = 4;
        exceptType->copyFunction = nullptr;
    }

}

/**
 * Throws an Objective-C exception.  This function is, unfortunately, used for
 * rethrowing caught exceptions too, even in @finally() blocks.  Unfortunately,
 * this means that we have some problems if the exception is boxed.
 */
extern "C" void objc_exception_throw(id object) {

    SEL rethrow_sel = sel_registerName("rethrow");
    if ((nil != object) &&
        (class_respondsToSelector(object_getClass(object), rethrow_sel))) {
        DEBUG_LOG("Rethrowing\n");
        IMP rethrow = objc_msg_lookup(object, rethrow_sel);
        rethrow(object, rethrow_sel);
        // Should not be reached!  If it is, then the rethrow method actually
        // didn't, so we throw it normally.
    }

    SEL processException_sel = sel_registerName("_processException");
    if ((nil != object) &&
        (class_respondsToSelector(object_getClass(object), processException_sel))) {
        IMP processException = objc_msg_lookup(object, processException_sel);
        processException(object, processException_sel);
    }

    DEBUG_LOG("Throwing %p\n", object);

    // The 'id' base type will be taking up a spot in the list:
    size_t typeCount = 1;

    // Get count of all types in exception
    for (Class curType = object_getClass(object); curType != nil; curType = class_getSuperclass(curType), ++typeCount) {
    }

    // The internal EH percolation assumes this is the vtable:
    const void* vtable = *(void**)&typeid(void *);

    // Unfortunately we can't put this in a real function since the alloca has to be in this stack frame:
#define CREATE_TYPE_DESCRIPTOR(desc, symName) \
    desc = reinterpret_cast<_TypeDescriptor*>(alloca(sizeof(_TypeDescriptor) + symName.size() + 1 /* null terminator */)); \
    desc->pVFTable = vtable; \
    desc->spare = nullptr; \
    strcpy_s(desc->name, symName.size() + 1, symName.c_str());

    auto exceptTypes =
        (__ObjC_CatchableTypeArray*)_alloca(sizeof(__ObjC_CatchableTypeArray) + sizeof(__ObjC_CatchableType*) * typeCount);
    exceptTypes->count = typeCount;

    //  Add exception type and all base types to throw information
    size_t curTypeIndex = 0;
    for (Class curType = object_getClass(object); curType != nil; curType = class_getSuperclass(curType)) {
        auto exceptType = exceptTypes->types[curTypeIndex++] = (__ObjC_CatchableType*)_alloca(sizeof(__ObjC_CatchableType));
        fillCatchableType(exceptType);

        auto mangledName = mangleClassName(curType);
        CREATE_TYPE_DESCRIPTOR(exceptType->type, mangledName);
    }

    //  Add id
    auto exceptType = exceptTypes->types[curTypeIndex] = (__ObjC_CatchableType*)_alloca(sizeof(__ObjC_CatchableType));
    fillCatchableType(exceptType);
    CREATE_TYPE_DESCRIPTOR(exceptType->type, std::string(".PAAAPAUobjc_object@@"));

    _ThrowInfo ti = { 0, NULL, NULL, (_CatchableTypeArray*)exceptTypes };

    _CxxThrowException(&object, &ti);

    DEBUG_LOG("Throw returned!\n");
    abort();
}
