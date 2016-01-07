#include <stdint.h>

#define SMALLOBJ_MASK 1
#define OBJECT_STACK_OFFSET 4

struct objc_slot {
    struct objc_class* owner;
    struct objc_class* cachedFor;
    const char* types;
    int version;
    void* method;
    void* selector;
};

struct dtable_t {
    uint32_t shift;
    uint32_t refCount;
    void* data[256];
};

struct objc_class {
    struct objc_class* isa;
    struct objc_class* super_class;
    const char* name;
    long version;
    unsigned long info;
    long instance_size;
    void* ivars;
    void* methods;
    struct dtable_t* dtable;
};

struct objc_object {
    struct objc_class* isa;
};

extern "C" struct objc_class* SmallObjectClasses[];
extern "C" void* slowMsgLookup();
extern "C" void* _NSConcreteGlobalBlock;

extern "C" __declspec(naked) void* objc_msgSend(struct objc_class* self, void* _cmd) {
#define OBJECT_STACK_OFFSET 4
#define SELECTOR_STACK_OFFSET 8
#include "objc_msgSend.x86-32.masm.inl"
}

extern "C" __declspec(naked) double objc_msgSend_fpret(struct objc_class* self, void* _cmd) {
#define OBJECT_STACK_OFFSET 4
#define SELECTOR_STACK_OFFSET 8
#define FPRET 1
#include "objc_msgSend.x86-32.masm.inl"
}

extern "C" __declspec(naked) void* objc_msgSend_stret(void* stret, struct objc_class* self, void* _cmd) {
#define OBJECT_STACK_OFFSET 8
#define SELECTOR_STACK_OFFSET 12
#include "objc_msgSend.x86-32.masm.inl"
}
