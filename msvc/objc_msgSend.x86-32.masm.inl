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

#line DBG_SKIP
__asm {
        // self (off the stack)
#line DBG_SKIP
        mov         eax, [esp + OBJECT_STACK_OFFSET]

        // nil check
#line DBG_SKIP
        test        eax, eax
#line DBG_SKIP
        je          returnNil

        // check if the object is "small" (a tagged pointer)
#line DBG_SKIP
        test        eax, SMALLOBJ_MASK
#line DBG_SKIP
        jne         smallObject

        // cls = self->isa
#line DBG_SKIP
        mov         eax, [eax] objc_object.isa

classLoaded:
#line DBG_SKIP
        lea         edx, [_NSConcreteGlobalBlock]
#line DBG_SKIP
        mov         ecx, [eax]
#line DBG_SKIP
        cmp         ecx, edx
#line DBG_SKIP
        jne         doubleDereferencedGlobalBlock
#line DBG_SKIP
        mov         eax, ecx

doubleDereferencedGlobalBlock:
        // _cmd (stack)
#line DBG_SKIP
        mov         ecx, [esp + SELECTOR_STACK_OFFSET]

        // load the class's dtable
#line DBG_SKIP
        mov         eax, [eax] objc_class.dtable

        // load the selector's uid
#line DBG_SKIP
        mov         ecx, [ecx]

        // (shift = dtable size - 8bit, 16bit, 32bit)
#line DBG_SKIP
        mov         edx, [eax] SparseArray.shift

#line DBG_SKIP
        cmp         edx, 8
#line DBG_SKIP
        je          dtable16
#line DBG_SKIP
        cmp         edx, 0
#line DBG_SKIP
        je          dtable8

        // dtable is 24 bit - its entries point to 16-bit dtables
#line DBG_SKIP
        mov         edx, ecx
#line DBG_SKIP
        shr         edx, 10h

        // 4* here for sizeof(void)
#line DBG_SKIP
        mov         eax, [eax] SparseArray.data[4*edx]
dtable16:   // dtable is 16-bit - its entries point to 8-bit dtables
#line DBG_SKIP
        movzx       edx, ch
#line DBG_SKIP
        mov         eax, [eax] SparseArray.data[4*edx]
dtable8:    // dtable is 8-bit - its entries point to objc_slots
#line DBG_SKIP
        movzx       edx, cl
#line DBG_SKIP
        mov         eax, [eax] SparseArray.data[4*edx]

#line DBG_SKIP
        // Did we get a nil method? If so, slow send (forwarding)
#line DBG_SKIP
        test        eax, eax
#line DBG_SKIP
        je          slowSend

        // dispatch
#line DBG_SKIP
        mov         eax, [eax] objc_slot.method
#line DBG_FOLLOW
        jmp         eax

returnNil:
#if defined FPRET
#line DBG_SKIP
        fldz
#else
#line DBG_SKIP
        xor         eax, eax
#line DBG_SKIP
        xor         edx, edx
#endif
#line DBG_SKIP
        ret

slowSend:
#line DBG_SKIP
        mov         ecx, [esp + SELECTOR_STACK_OFFSET]
#line DBG_SKIP
        lea         eax, [esp + OBJECT_STACK_OFFSET]

        // slowMsgLookup may modify self to change the destination
#line DBG_SKIP
        push        ecx
#line DBG_SKIP
        push        eax
#line DBG_SKIP
        call        slowMsgLookup // slowMsgLookup(&self, _cmd)

        // restore the stack
#line DBG_SKIP
        add         esp, 8

        // dispatch
#line DBG_FOLLOW
        jmp         eax

smallObject:
#line DBG_SKIP
        lea         eax, SmallObjectClasses
#line DBG_SKIP
        mov         eax, [eax]
#line DBG_SKIP
        jmp         classLoaded
}

#if defined FPRET
#undef FPRET
#endif
#undef OBJECT_STACK_OFFSET
#undef SELECTOR_STACK_OFFSET
