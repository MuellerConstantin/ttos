# TTOS

> Just another Tiny Toy Operating System (TTOS) for learning purposes.

![](https://img.shields.io/badge/C-gray?logo=c)
![](https://img.shields.io/badge/Qemu-orange?logo=qemu&logoColor=white)
![](https://img.shields.io/badge/Intel%20x86-blue?logo=intel)

---

- [Introduction](#introduction)
- [Build Instructions](#build-instructions)
  - [Requirements](#requirements)
  - [Build Process](#build-process)
- [License](#license)
  - [Forbidden](#forbidden)

---

## Introduction

Welcome to the *Tiny Toy Operating System (TTOS)* project. This project is
intended to be a simple operating system for learning purposes. It is written
for the Intel x86 architecture and originally aimed at gaining a deeper
understanding of the internal processes of an operating system as well as the
functionality of the hardware. The kernel is written in C and Assembly and
is [multiboot](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
compliant, which means it can be booted by the [GRUB](https://www.gnu.org/software/grub/)
bootloader.

The project itself is divided into three main parts:

- **Kernel**: The core of the operating system. It contains the source code
  for all core functionalities and default drivers of the operating system. See
  [here](kernel/README.md) for more information.
- **libc**: A naive implementation of a standard C library for the TTOS project.
  It may be replaced by an existing C library implementation/port in the future.
  See [here](libc/README.md) for more information.
- **libk**: A special library that bundles kernel-specific functions and utilities
  used by the TTOS kernel. It is an extension of the C standard library.
  See [here](libk/README.md) for more information.

## Build Instructions

The TTOS Project is intended to be built using the GNU toolchain with the GNU Compiler
Collection (GCC) and the Netwide Assembler (NASM) on a Unix-like system, but similar
configurations might also work. In addition, [GRUB](https://www.gnu.org/software/grub/)
is used to boot the operating system and create a bootable image. The build process
is automated using a GNU Make and corresponding Makefiles.

### Requirements

- **GNU Make**: The GNU Make utility is used to build the TTOS project. It is
  required to run the build process using `make`.
- **GCC**: The GNU Compiler Collection is a collection of compilers for various
  programming languages. It is the default compiler for the TTOS project.
- **NASM**: The Netwide Assembler is an assembler and disassembler for the Intel
  x86 architecture. It is used to compile the assembly code of the TTOS project.
- **GRUB**: The GNU GRUB (GRand Unified Bootloader) is a multiboot compliant
  bootloader that is used to boot the TTOS kernel. For creating a bootable image,
  GRUB command line tools, especially `grub-mkrescue`, are required.

### Build Process

As mentioned before, the build process is automated using a GNU Make and
corresponding Makefiles. Hence, building the TTOS project is as simple as
running the `make` command in the root directory of the project. The following
commands are available:

- **`make all`**: Builds the TTOS project, this includes building the kernel,
  the libc, and the libk library, and creating a bootable image. This results
  in a bootable image that is placed at `<ROOTDIR>/ttos-<VERSION>-intel-x86.iso`.
- **`make kernel`**: Builds the kernel of the TTOS project without creating a
  bootable image. This results in a kernel binary that is placed at
  `<ROOTDIR>/kernel/kernel.elf`.
- **`make clean`**: Cleans the build directory and removes all generated files.

## License

Copyright (c) 2024 Constantin Müller

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

[MIT License](https://opensource.org/licenses/MIT) or [LICENSE](LICENSE) for
more details.

### Forbidden

**Hold Liable**: Software is provided without warranty and the software
author/license owner cannot be held liable for damages.
