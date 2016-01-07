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

using BOOL = char;
using id = void*;
using Class = id;
using SEL = char*;
using IMP = id(*)(id, SEL);

#define nil ((id)nullptr)

extern "C" SEL sel_registerName(const char*);
extern "C" bool class_respondsToSelector(Class, SEL);
extern "C" IMP objc_msg_lookup(id, SEL);
extern "C" Class object_getClass(id);
extern "C" Class class_getSuperclass(Class);
extern "C" const char* class_getName(Class);

typedef struct {
    unsigned int flags;
    const char* type;
    int nonVirtualAdjustment;
    int vbPtrOffset;
    int vTableIndex;
    int size;
    void* copyFunction;
} __ObjC_CatchableType;

typedef struct {
    int count;
    __ObjC_CatchableType* types[0];
} __ObjC_CatchableTypeArray;

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

    DEBUG_LOG("Throwing %p\n", object);

    std::vector<__ObjC_CatchableType*> _catchableTypes;
    Class curType = object_getClass(object);
    while (curType) {
        _catchableTypes.push_back(new __ObjC_CatchableType{
            1, class_getName(curType), 0, -1, 0, sizeof(Class)
        });
        curType = class_getSuperclass(curType);
    }
    _catchableTypes.push_back(new __ObjC_CatchableType{
        1, "id", 0, -1, 0, sizeof(Class)
    });

    __ObjC_CatchableTypeArray* exceptTypes = (__ObjC_CatchableTypeArray*)calloc(sizeof(int) + sizeof(__ObjC_CatchableType) * _catchableTypes.size(), 1);
    exceptTypes->count = _catchableTypes.size();
    memcpy(&exceptTypes->types[0], _catchableTypes.data(), _catchableTypes.size() * sizeof(__ObjC_CatchableType*));

    _ThrowInfo ti = { 0, NULL, NULL, (_CatchableTypeArray*)exceptTypes };

    _CxxThrowException(&object, &ti);

    DEBUG_LOG("Throw returned!\n");
	abort();
}
