ENTRY(kstart)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

physical_kernel_address = 0x100000;

SECTIONS {
	.text physical_kernel_address : AT(physical_kernel_address) {
		code = .;
		*(.multiboot)
		*(.text)
		. = ALIGN(4096);
	}

	.data : AT(physical_kernel_address + (data - code)) {
		data = .;
		*(.data)
		*(.rodata)
		. = ALIGN(4096);
	}

    .rodata : AT(physical_kernel_address + (rodata - code)) {
        rodata = .;
        *(.rodata)
        . = ALIGN(4096);
    }

	.bss : AT(physical_kernel_address + (bss - code)) {
		bss = .;
		*(.bss)
		. = ALIGN(4096);
	}

	end = .;
}
