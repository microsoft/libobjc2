#include <stdint.h>
#include "../sarray2.h"
#include "debug_constants.h"

#define SMALLOBJ_MASK 1
#define OBJECT_STACK_OFFSET 4

struct objc_ivar_list;
struct objc_method_list;
struct objc_protocol_list;
struct reference_list;
struct objc_property_list;

struct objc_class {
    struct objc_class         *isa;
    struct objc_class         *super_class;
    const char                *name;
    long                       version;
    unsigned long              info;
    long                       instance_size;
    struct objc_ivar_list     *ivars;
    struct objc_method_list   *methods;
    void                      *dtable;
    struct objc_class         *subclass_list;
    struct objc_class         *sibling_class;
    struct objc_protocol_list *protocols;
    struct reference_list     *extra_data;
    long                       abi_version;
    int                      **ivar_offsets;
    struct objc_property_list *properties;
    uintptr_t                  strong_pointers;
    uintptr_t                  weak_pointers;
};

struct objc_object {
    struct objc_class* isa;
};

struct objc_selector;
typedef struct objc_object* id;
typedef struct objc_selector* SEL;
typedef id(*IMP)(id, SEL, ...);

struct objc_slot {
    struct objc_class* owner;
    struct objc_class* cachedFor;
    const char* types;
    int version;
    IMP method;
    SEL selector;
};

extern "C" struct objc_class* SmallObjectClasses[];
extern "C" IMP slowMsgLookup();
extern "C" struct objc_class _NSConcreteGlobalBlock;

#line DBG_SKIP
extern "C" __declspec(naked) void* objc_msgSend(struct objc_class* self, void* _cmd) {
#define OBJECT_STACK_OFFSET 4
#define SELECTOR_STACK_OFFSET 8
#include "objc_msgSend.x86-32.masm.inl"
}

#line DBG_SKIP
extern "C" __declspec(naked) double objc_msgSend_fpret(struct objc_class* self, void* _cmd) {
#define OBJECT_STACK_OFFSET 4
#define SELECTOR_STACK_OFFSET 8
#define FPRET 1
#include "objc_msgSend.x86-32.masm.inl"
}

#line DBG_SKIP
extern "C" __declspec(naked) void* objc_msgSend_stret(void* stret, struct objc_class* self, void* _cmd) {
#define OBJECT_STACK_OFFSET 8
#define SELECTOR_STACK_OFFSET 12
#include "objc_msgSend.x86-32.masm.inl"
}
