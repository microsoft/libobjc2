#include <kxarm.h>
#include "../asmconstants.h"
#include "debug_constants.h"

    TEXTAREA

    EXTERN _NSConcreteGlobalBlock

#line DBG_SKIP
    MACRO
    MSGSEND $receiver, $sel
#line DBG_SKIP

#line DBG_SKIP
    PROLOG_PUSH {r4-r7, r11, lr}           ; We're going to use some scratch registers,
                                           ; so save them now.

#line DBG_SKIP
    cbz   $receiver, %3f                   ; Skip everything if the receiver is nil

#line DBG_SKIP
    tst   $receiver, #SMALLOBJ_MASK        ; Sets Z if this is not a small int

#line DBG_SKIP
    ldr   r4, LSmallIntClass               ; Small Int class -> r4 if this is a small int
#line DBG_SKIP
    moveq r4, $receiver                    ; Load class to r4 if not a small int
#line DBG_SKIP
    ldr   r4, [r4]

#line DBG_SKIP
    ldr   r5, [r4]                         ; If the class is NSConcreteGlobalBlock,
#line DBG_SKIP
    ldr   r6, =_NSConcreteGlobalBlock      ; we have to double-dereference the pointer
#line DBG_SKIP
    cmp   r5, r6
#line DBG_SKIP
    moveq r4, r5

#line DBG_SKIP
    ldr   r4, [r4, #DTABLE_OFFSET]         ; Dtable -> r4

#line DBG_SKIP
    ldr   r5, [$sel]                       ; selector->index -> r5

#line DBG_SKIP
    ldr   r6, [r4, #SHIFT_OFFSET]          ; dtable->shift -> r6

#line DBG_SKIP
    cmp   r6, #8                           ; If this is a small dtable, jump to the small dtable handlers
#line DBG_SKIP
    beq   %1f
#line DBG_SKIP
    cbz   r6, %2f

#line DBG_SKIP
    ubfx  r6, r5, #16, #8                  ; Put byte 3 of the sel id in r6
#line DBG_SKIP
    add   r6, r4, r6, lsl #2               ; r6 = dtable address + dtable data offset
#line DBG_SKIP
    ldr   r4, [r6, #DATA_OFFSET]           ; Load, adding in the data offset

1                                          ; dtable16
#line DBG_SKIP
    ubfx  r6, r5, #8, #8                   ; Put byte 2 of the sel id in r6
#line DBG_SKIP
    add   r6, r4, r6, lsl #2               ; r6 = dtable address + dtable data offset
#line DBG_SKIP
    ldr   r4, [r6, #DATA_OFFSET]           ; Load, adding in the data offset
2                                          ; dtable8
#line DBG_SKIP
    uxtb  r6, r5                           ; Low byte of sel id into r6
#line DBG_SKIP
    add   r6, r4, r6, lsl #2               ; r6 = dtable address + dtable data offset
#line DBG_SKIP
    ldr   r6, [r6, #DATA_OFFSET]           ; Load, adding in the data offset

#line DBG_SKIP
    cbz   r6, %4f                          ; If the slot is nil, go to the slow path and do the forwarding stuff

#line DBG_SKIP
    ldr   ip, [r6, #SLOT_OFFSET]           ; Load the method from the slot
#line DBG_SKIP
    b     %5f

3                                          ; Nil receiver
#line DBG_SKIP
    movs  r0, #0
#line DBG_SKIP
    movs  r1, #0
#line DBG_SKIP
    mov   ip, lr
#line DBG_SKIP
    b     %5f

4                                          ; Slow lookup
#line DBG_SKIP
    mov   r4, $receiver
#line DBG_SKIP
    mov   r5, $sel
#line DBG_SKIP
    bl    doSlowMsgLookup
#line DBG_SKIP
    mov   $receiver, r4

5
#line DBG_SKIP
    EPILOG_POP {r4-r7, r11, lr}            ; Restore the saved callee-save registers
#line DBG_FOLLOW
    EPILOG_BRANCH_REG ip

    MEND

    ; Custom calling convention on this function.
    ; Arguments:
    ;    r4 = receiver
    ;    r5 = sel
    ; Return:
    ;    r4 = receiver
    ;    ip = IMP
    NESTED_ENTRY doSlowMsgLookup
#line DBG_SKIP
    PROLOG_PUSH {r0-r3, r11, lr}
#line DBG_SKIP
    PROLOG_VPUSH {d0-d7}
#line DBG_SKIP
    PROLOG_STACK_ALLOC 8

#line DBG_SKIP
    str   r4, [sp]                         ; &self, _cmd in arguments

#line DBG_SKIP
    mov   r0, sp
#line DBG_SKIP
    mov   r1, r5

#line DBG_SKIP
    EXTERN slowMsgLookup
#line DBG_SKIP
    bl    slowMsgLookup                    ; This is the only place where the CFI directives have to be accurate...
#line DBG_SKIP
    mov   ip, r0                           ; IMP -> ip

#line DBG_SKIP
    ldr   r4, [sp]                         ; restore (modified) self to r4

#line DBG_SKIP
    EPILOG_STACK_FREE 8
#line DBG_SKIP
    EPILOG_VPOP {d0-d7}
#line DBG_SKIP
    EPILOG_POP {r0-r3, r11, pc}
#line DBG_SKIP
    NESTED_END doSlowMsgLookup

#line DBG_SKIP
    NESTED_ENTRY objc_msgSend
#line DBG_SKIP
    ALTERNATE_ENTRY objc_msgSend_fpret
#line DBG_FOLLOW
    MSGSEND r0, r1
#line DBG_FOLLOW
    NESTED_END objc_msgSend

#line DBG_SKIP
    NESTED_ENTRY objc_msgSend_stret
#line DBG_FOLLOW
    MSGSEND r1, r2
#line DBG_FOLLOW
    NESTED_END objc_msgSend_stret

    EXTERN SmallObjectClasses
LSmallIntClass DCD SmallObjectClasses

#line DBG_SKIP
    END