bits 64
section .text

global SetSegmentRegistor
SetSegmentRegistor:
    mov ss, di
    mov ds, di
    mov es, di
    mov fs, di
    mov gs, di
    ret

global SetCS
SetCS:
    push rbp
    mov rbp, rsp
    mov rax, .next
    push rdi
    push rax
    o64 retf
.next:
    mov rsp, rbp
    pop rbp
    ret