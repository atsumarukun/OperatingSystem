bits 64
section .text

extern kernel_stack
extern Main

global CallKernel
CallKernel:
    mov rsp, kernel_stack + 1024 * 1024
    call Main
.fin:
    hlt
    jmp .fin