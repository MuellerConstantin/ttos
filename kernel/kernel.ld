ENTRY(kstart)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

kernel_physical_address = 0x00100000;
kernel_virtual_address = 0xC0100000;

SECTIONS {
	. = kernel_virtual_address;

	kernel_virtual_start = .;
	kernel_physical_start = kernel_physical_address;

	.text ALIGN(4096) : AT(ADDR(.text) - 0xC0000000) {
		*(.multiboot)
		*(.text)
	}

	.data ALIGN(4096) : AT(ADDR(.data) - 0xC0000000) {
		*(.data)
	}

    .rodata ALIGN(4096) : AT(ADDR(.rodata) - 0xC0000000){
        *(.rodata)
    }

	.bss ALIGN(4096) : AT(ADDR(.bss) - 0xC0000000) {
		*(.bss)
	}

	kernel_virtual_end = .;
	kernel_physical_end = kernel_physical_address + (kernel_virtual_end - kernel_virtual_start);
}
