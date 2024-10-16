# TTOS - Kernel

This subproject contains the kernel of the Tiny Toy Operating System (TTOS). It
conntains the source code for all core functionalities and default drivers of
the operating system.

## Architecture

The kernel is written in C and Assembly and requires an Intel x86 architecture
32 bit or higher. Primarily, it targets the Intel i440fx chipset combined with
an Intel Pentium processor, but it should be compatible with other similar
configurations. It is [multiboot](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
compliant, which means it must be booted by a multiboot compliant bootloaders like
[GRUB](https://www.gnu.org/software/grub/). Moreover, the kernel is written as a
monolithic kernel and hence does contain all core functionalities and drivers in
a single binary.
