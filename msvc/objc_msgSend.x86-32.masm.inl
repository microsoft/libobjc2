/*
 * This is, largely, a translation of \objc_msgSend.x86-32.S into MASM.
 * It adds support for the WinObjC compiler's incorrect view of block globals by
 * double-dereferencing the isa of every object.
 */

#ifndef OBJECT_STACK_OFFSET
#error OBJECT_STACK_OFFSET not defined
#endif
#ifndef SELECTOR_STACK_OFFSET
#error SELECTOR_STACK_OFFSET not defined
#endif

__asm {
        // self (off the stack)
        mov         eax, [esp + OBJECT_STACK_OFFSET]

        // nil check
        test        eax, eax
        je          returnNil

        // check if the object is "small" (a tagged pointer)
        test        eax, SMALLOBJ_MASK
        jne         smallObject

        // cls = self->isa
        mov         eax, [eax] objc_object.isa

classLoaded:
        lea         edx, [_NSConcreteGlobalBlock]
        mov         ecx, [eax]
        cmp         ecx, edx
        jne         doubleDereferencedGlobalBlock
        mov         eax, ecx

doubleDereferencedGlobalBlock:
        // _cmd (stack)
        mov         ecx, [esp + SELECTOR_STACK_OFFSET]

        // load the class's dtable
        mov         eax, [eax] objc_class.dtable

        // load the selector's uid
        mov         ecx, [ecx]

        // (shift = dtable size - 8bit, 16bit, 32bit)
        mov         edx, [eax] dtable_t.shift

        cmp         edx, 8
        je          dtable16
        cmp         edx, 0
        je          dtable8

        // dtable is 24 bit - its entries point to 16-bit dtables
        mov         edx, ecx
        shr         edx, 10h
        // 4* here for sizeof(void)
        mov         eax, [eax] dtable_t.data[4*edx]
dtable16:   // dtable is 16-bit - its entries point to 8-bit dtables
        movzx       edx, ch
        mov         eax, [eax] dtable_t.data[4*edx]
dtable8:    // dtable is 8-bit - its entries point to objc_slots
        movzx       edx, cl
        mov         eax, [eax] dtable_t.data[4*edx]

        // Did we get a nil method? If so, slow send (forwarding)
        test        eax, eax
        je          slowSend

        // dispatch
        mov         eax, [eax] objc_slot.method
        jmp         eax

returnNil:
#if defined FPRET
        fldz
#else
        xor         eax, eax
        xor         edx, edx
#endif
        ret

slowSend:
        mov         ecx, [esp + SELECTOR_STACK_OFFSET]
        lea         eax, [esp + OBJECT_STACK_OFFSET]

        // slowMsgLookup may modify self to change the destination
        push        ecx
        push        eax
        call        slowMsgLookup // slowMsgLookup(&self, _cmd)

        // restore the stack
        add         esp, 8

        // dispatch
        jmp         eax

smallObject:
        lea         eax, SmallObjectClasses
        mov         eax, [eax]
        jmp         classLoaded
}

#if defined FPRET
#undef FPRET
#endif
#undef OBJECT_STACK_OFFSET
#undef SELECTOR_STACK_OFFSET
