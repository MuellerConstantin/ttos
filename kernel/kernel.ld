ENTRY(kstart)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

kernel_physical_address = 0x100000;

SECTIONS {
	kernel_start = kernel_physical_address;

	.text kernel_physical_address : AT(kernel_physical_address) {
		code = .;
		*(.multiboot)
		*(.text)
		. = ALIGN(4096);
	}

	.data : AT(kernel_physical_address + (data - code)) {
		data = .;
		*(.data)
		*(.rodata)
		. = ALIGN(4096);
	}

    .rodata : AT(kernel_physical_address + (rodata - code)) {
        rodata = .;
        *(.rodata)
        . = ALIGN(4096);
    }

	.bss : AT(kernel_physical_address + (bss - code)) {
		bss = .;
		*(.bss)
		. = ALIGN(4096);
	}

	kernel_end = .;
}
