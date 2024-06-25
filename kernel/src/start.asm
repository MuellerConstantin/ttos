[BITS 32]

MBOOT_PAGE_ALIGN	equ 0x01 << 0x00
MBOOT_MEM_INFO      equ 0x01 << 0x01
MBOOT_HEADER_MAGIC	equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

KSTACK_SIZE equ 4096

[SECTION .multiboot]

dd  MBOOT_HEADER_MAGIC
dd  MBOOT_HEADER_FLAGS
dd  MBOOT_CHECKSUM

[SECTION .bss]

kstack:

	resb KSTACK_SIZE

[SECTION .text]

[GLOBAL kstart]
[EXTERN kmain]

kstart:

	; Setup tentative stack
	mov esp, kstack + KSTACK_SIZE

	; Save multiboot magic number
	push eax
	; Save pointer to multiboot info
	push ebx

	cli
	call kmain
	hlt
