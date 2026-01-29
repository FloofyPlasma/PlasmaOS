section .text
bits 64

; Load GDT and reload segment registers
; void gdt_load(gdt_ptr_t *gdt_ptr);
global gdt_load
gdt_load:
    lgdt [rdi]

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    pop rdi
    mov rax, 0x08
    push rax
    push rdi
    retfq

; Load TSS
; void tss_load(uint16_t tss_selector);
global tss_load
tss_load:
    ltr di
    ret