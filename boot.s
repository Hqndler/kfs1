bits 32

ALIG                equ 1<<0 ; align loaded modules on page boundaries
MEMINFO             equ 1<<1 ; provide memory map
FLAGS               equ ALIG | MEMINFO ; multiboot flags
MAGIC_NUMBER        equ 0x1BADB002 ; define the magic number constant
CHECKSUM            equ -(MAGIC_NUMBER + FLAGS) ; calculate the checksum
KERNEL_STACK_SIZE   equ 8192 ; size of stack in bytes

section .multiboot
align 4
    dd MAGIC_NUMBER
    dd FLAGS
    dd CHECKSUM

section .bss
align 16

stack_bottom:
resb KERNEL_STACK_SIZE
stack_top:

section .text
global _start:function (_start.end - _start)
_start:
    mov esp, stack_top
    extern kernel_main
    call kernel_main
    cli
.hang:	hlt
    jmp .hang
.end:


