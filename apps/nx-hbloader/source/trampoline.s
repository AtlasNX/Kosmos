.section .text.nroEntrypointTrampoline, "ax", %progbits

.global nroEntrypointTrampoline
.type   nroEntrypointTrampoline, %function
.align 2

.cfi_startproc

nroEntrypointTrampoline:

    // Reset stack pointer.
    adrp x8, __stack_top //Defined in libnx.
    ldr  x8, [x8, #:lo12:__stack_top]
    mov  sp, x8

    // Call NRO.
    blr  x2

    // Save retval
    adrp x1, g_lastRet
    add  x1, x1, #:lo12:g_lastRet
    str  x0, [x1]

    // Reset stack pointer and load next NRO.
    adrp x8, __stack_top
    ldr  x8, [x8, #:lo12:__stack_top]
    mov  sp, x8

    b    loadNro

.cfi_endproc
