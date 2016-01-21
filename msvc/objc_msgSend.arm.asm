#include <kxarm.h>
#include "../asmconstants.h"

    TEXTAREA

    EXTERN _NSConcreteGlobalBlock

    MACRO
    MSGSEND $receiver, $sel

    PROLOG_PUSH {r4-r7, r11, lr}           ; We're going to use some scratch registers,
                                           ; so save them now.

    cbz   $receiver, %3f                   ; Skip everything if the receiver is nil

    tst   $receiver, #SMALLOBJ_MASK        ; Sets Z if this is not a small int

    ldr   r4, LSmallIntClass               ; Small Int class -> r4 if this is a small int
    moveq r4, $receiver                    ; Load class to r4 if not a small int
    ldr   r4, [r4]

    ldr   r5, [r4]                         ; If the class is NSConcreteGlobalBlock,
    ldr   r6, =_NSConcreteGlobalBlock      ; we have to double-dereference the pointer
    cmp   r5, r6
    moveq r4, r5

    ldr   r4, [r4, #DTABLE_OFFSET]         ; Dtable -> r4

    ldr   r5, [$sel]                       ; selector->index -> r5

    ldr   r6, [r4, #SHIFT_OFFSET]          ; dtable->shift -> r6

    cmp   r6, #8                           ; If this is a small dtable, jump to the small dtable handlers
    beq   %1f
    cbz   r6, %2f

    ubfx  r6, r5, #16, #8                  ; Put byte 3 of the sel id in r6
    add   r6, r4, r6, lsl #2               ; r6 = dtable address + dtable data offset
    ldr   r4, [r6, #DATA_OFFSET]           ; Load, adding in the data offset

1                                          ; dtable16
    ubfx  r6, r5, #8, #8                   ; Put byte 2 of the sel id in r6
    add   r6, r4, r6, lsl #2               ; r6 = dtable address + dtable data offset
    ldr   r4, [r6, #DATA_OFFSET]           ; Load, adding in the data offset
2                                          ; dtable8
    uxtb  r6, r5                           ; Low byte of sel id into r6
    add   r6, r4, r6, lsl #2               ; r6 = dtable address + dtable data offset
    ldr   r6, [r6, #DATA_OFFSET]           ; Load, adding in the data offset

    cbz   r6, %4f                          ; If the slot is nil, go to the slow path and do the forwarding stuff

    ldr   ip, [r6, #SLOT_OFFSET]           ; Load the method from the slot
    b     %5f

3                                          ; Nil receiver
    movs  r0, #0
    movs  r1, #0
    mov   ip, lr
    b     %5f

4                                          ; Slow lookup
    mov   r4, $receiver
    mov   r5, $sel
    bl    doSlowMsgLookup
    mov   $receiver, r4

5
    EPILOG_POP {r4-r7, r11, lr}            ; Restore the saved callee-save registers
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
    PROLOG_PUSH {r0-r3, r11, lr}
    PROLOG_VPUSH {d0-d7}
    PROLOG_STACK_ALLOC 8

    str   r4, [sp]                         ; &self, _cmd in arguments

    mov   r0, sp
    mov   r1, r5

    EXTERN slowMsgLookup
    bl    slowMsgLookup                    ; This is the only place where the CFI directives have to be accurate...
    mov   ip, r0                           ; IMP -> ip

    ldr   r4, [sp]                         ; restore (modified) self to r4

    EPILOG_STACK_FREE 8
    EPILOG_VPOP {d0-d7}
    EPILOG_POP {r0-r3, r11, pc}
    NESTED_END doSlowMsgLookup

    NESTED_ENTRY objc_msgSend
    ALTERNATE_ENTRY objc_msgSend_fpret
    MSGSEND r0, r1
    NESTED_END objc_msgSend

    NESTED_ENTRY objc_msgSend_stret
    MSGSEND r1, r2
    NESTED_END objc_msgSend_stret

    EXTERN SmallObjectClasses
LSmallIntClass DCD SmallObjectClasses

    END
