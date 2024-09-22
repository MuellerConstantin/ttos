[BITS 32]

MBOOT_PAGE_ALIGN	equ 0x01 << 0x00
MBOOT_MEM_INFO      equ 0x01 << 0x01
MBOOT_HEADER_MAGIC	equ 0x1BADB002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

KSTACK_SIZE equ 10240

KERNEL_VIRTUAL_BASE				equ 0xC0000000
KERNEL_DIRECTORY_ENTRY_INDEX	equ (KERNEL_VIRTUAL_BASE >> 22)

[SECTION .multiboot]

dd  MBOOT_HEADER_MAGIC
dd  MBOOT_HEADER_FLAGS
dd  MBOOT_CHECKSUM

[SECTION .bss]

kstack:

	resb KSTACK_SIZE

[SECTION .data]

[GLOBAL prepaging_page_directory]

; Temporary page directory to allow the kernel to be loaded as a higher half kernel
; before the proper page directory is set up. The page directory uses 4MB pages for
; simplicity and to avoid the need for page tables during the prepaging stage.

prepaging_page_directory:
	; Identity map the first 4MB of virtual memory to the first 4MB of physical memory
    dd 0x00000083
	; Fill the unused directory entries
    times(KERNEL_DIRECTORY_ENTRY_INDEX - 1) dd 0
	; Map the first 4MB of the higher half kernel (virtual memory) to the first 4MB of physical memory
    dd 0x00000083
	; Fill remaining unused directory entries
    times(1024 - KERNEL_DIRECTORY_ENTRY_INDEX - 1) dd 0

[SECTION .text]

[GLOBAL kstart]
[EXTERN kmain]

kstart:
	; Point to physical address of prepaging page directory
	mov ecx, (prepaging_page_directory - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx

    ; Enable 4MB pages without page tables during prepaging
    mov ecx, cr4;
    or ecx, 0x00000010
    mov cr4, ecx

    ; Enable paging
    mov ecx, cr0
    or ecx, 0x80000000
    mov cr0, ecx

	; Long jump to higher half
    lea ecx, [khigher_half]
    jmp ecx

khigher_half:

	; Setup tentative stack
	mov esp, kstack + KSTACK_SIZE

	; Save multiboot magic number
	push eax
	; Save pointer to multiboot info
	push ebx

	cli
	call kmain
	hlt
