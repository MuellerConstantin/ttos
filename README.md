# TTOS

<pre>
  ________________  ____
 /_  __/_  __/ __ \/ __/
  / /   / / / /_/ /\ \
 /_/   /_/  \____/___/
Tiny Toy Operating System
</pre>

> Just another Tiny Toy Operating System (TTOS) for learning purposes.

![](https://img.shields.io/badge/C-gray?logo=c)
![](https://img.shields.io/badge/Qemu-orange?logo=qemu&logoColor=white)
![](https://img.shields.io/badge/Intel%20x86-blue?logo=intel)

---

- [Introduction](#introduction)
- [Build Instructions](#build-instructions)
  - [Requirements](#requirements)
  - [Build Process](#build-process)
- [Development](#development)
  - [Emulation](#emulation)
  - [Disk Images](#disk-images)
  - [Debugging](#debugging)
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

The project itself is divided into multiple parts:

- **kernel**: The core of the operating system. It contains the source code
  for all core functionalities and default drivers of the operating system. See
  [here](kernel/README.md) for more information.
- **libc**: A naive implementation of a standard C library for the TTOS project.
  It may be replaced by an existing C library implementation/port in the future.
  See [here](libc/README.md) for more information.
- **libsys**: An additional library that bundles system-level calls. With the help
  of this library, user programs can perform system-specific operations. See
  [here](libsys/README.md) for more information.
- **userland**: A subproject that contains the source code for userland applications
  provided by the operating system by default. See [here](userland/README.md) for
  more information.
- **abi**: The binary interface between the kernel and userland. It contains the
  definitions both sides have to agree on and consists of headers only. See
  [here](abi/README.md) for more information.

## Build Instructions

The TTOS Project is built using the GNU toolchain with the GNU Compiler Collection
(GCC) and the Netwide Assembler (NASM). In addition,
[GRUB](https://www.gnu.org/software/grub/) is used to boot the operating system and
create a bootable image. The build process is automated using a GNU Make and
corresponding Makefiles.

Both building and developing the project require a **Linux** system. The project does
not use a cross-compiler; it relies on the host GCC and GNU `ld` being able to target
32-bit ELF directly, and the disk images are assembled with Linux-specific tools
such as `mkfs.ext2` and `fdisk`. Running the build on WSL works as well and is what the project is
developed on. Other Unix-like systems, macOS in particular, are not supported without
providing a proper i686 cross-toolchain first.

### Requirements

- **Linux**: See above. A native installation as well as WSL will do.
- **GNU Make**: The GNU Make utility is used to build the TTOS project. It is
  required to run the build process using `make`.
- **GCC**: The GNU Compiler Collection is a collection of compilers for various
  programming languages. It is the default compiler for the TTOS project. Since the
  kernel and the userland are compiled as 32-bit code via `-m32`, the 32-bit target
  support has to be installed as well, which on 64-bit distributions is usually
  provided by a `gcc-multilib` package.
- **GNU Binutils**: The kernel and the userland binaries are linked with GNU `ld`
  using the `elf_i386` emulation, so a `ld` that supports this target is required.
- **NASM**: The Netwide Assembler is an assembler and disassembler for the Intel
  x86 architecture. It is used to compile the assembly code of the TTOS project.
- **GRUB**: The GNU GRUB (GRand Unified Bootloader) is a multiboot compliant
  bootloader that is used to boot the TTOS kernel. For creating a bootable image,
  GRUB command line tools are required: `grub-mkrescue`, which in turn relies on
  `xorriso` to author the ISO image, and `grub-mkimage`, which builds the boot
  image an installed disk is started from. The stock `boot.img` is taken from the
  GRUB installation itself.
- **Python 3**: The initial ramdisk, which is part of every bootable image, is packed
  by `scripts/mkinitrd.py` and therefore requires a Python 3 interpreter.

### Build Process

As mentioned before, the build process is automated using a GNU Make and
corresponding Makefiles. Hence, building the TTOS project is as simple as
running the `make` command in the root directory of the project. The following
commands are available:

- **`make all`**: Builds the TTOS project, this includes building the kernel,
  the libc, and the libsys library, and creating a bootable image. This results
  in a bootable image that is placed at `<ROOTDIR>/ttos-<VERSION>-intel-x86.iso`.
- **`make kernel`**: Builds the kernel of the TTOS project without creating a
  bootable image. This results in a kernel binary that is placed at
  `<ROOTDIR>/kernel/kernel.elf`.
- **`make clean`**: Cleans the build directory and removes all generated files.

## Development

For the most part, development requires no more tools than the build process.
However, to ensure smooth development, an emulator is necessary to allow for easy
testing of the boot process and the kernel. [QEMU](https://www.qemu.org/) is used for
this purpose in this project. Concretely, the following additional tools are expected
to be available on top of the [build requirements](#requirements):

- **QEMU**: The `qemu-system-i386` binary is used to emulate an Intel x86 machine
  and run the TTOS image without rebooting the development machine.
- **`mkfs.ext2`**: Both the system ramdisk and the partition of the installed disk
  are ext2 images populated directly from the staged system tree with
  `mkfs.ext2 -d`, which needs no privileges.
- **`fdisk`**: Writes the partition table of the disk image in `make hda-install`.
- **GDB**: Only required for source-level debugging of the running kernel, see
  [Debugging](#debugging).

### Emulation

Emulation is also controlled via Make; the following commands are available for
this purpose:

- **`make qemu-live`**: Builds the bootable image as well as the disk images and
  boots the live medium. GRUB loads the kernel, the initial ramdisk and the system
  ramdisk `systemrd.img` from the emulated CD-ROM; the system runs from the latter,
  a writable ext2 image in memory. `hda.img` is attached as the disk an
  installation would go onto.
- **`make qemu-installed`**: Boots the installed system. No CD-ROM is attached at
  all; GRUB is loaded from the MBR of `hda.img` and pulls the kernel and the initial
  ramdisk from the ext2 partition of that disk. This is the way to verify that an
  installation actually boots on its own. On a blank disk the boot ends at the
  firmware's boot failure, as it would on a real machine.
- **`make qemu`**: A shorthand for `make qemu-live`.
- **`make qemu-debug`**: Same as `make qemu-live`, but starts QEMU with a GDB stub
  attached and the CPU halted, see [Debugging](#debugging).

### Disk Images

Two disk images are used by the emulation targets and are created on demand:

- **`hda.img`**: The disk attached to the emulated IDE controller. It is created
  blank and keeps whatever is written to it, so a system installed from a live
  session survives into the next `make qemu-installed`.
- **`sda.img`**: A blank disk attached to the emulated AHCI controller, used to
  exercise the corresponding driver and the volume and filesystem layers.

Both images are 50 MiB in size and are *not* removed by `make clean`. They are
managed by their own targets instead:

- **`make qemu-disk`**: Creates the disk images without starting the emulator.
- **`make qemu-clean`**: Removes both disk images, which is the way back to a
  blank disk.
- **`make hda-install`**: Installs the system tree onto `hda.img` from the host:
  partitions the disk, writes the system tree onto its ext2 partition and places
  the GRUB boot code in front of it. This is the reference for what an installed
  disk has to look like, independent of any installer running inside the system,
  and it writes the same bytes such an installer would have to write.

### Debugging

Running `make qemu-debug` starts the emulator with the QEMU GDB stub listening on
`localhost:1234` and the guest CPU halted before the first instruction is executed.
This leaves enough time to attach a debugger and to place breakpoints before the
bootloader hands control over to the kernel.

The repository ships a `.gdbinit` that connects to the stub, loads the symbols from
the unstripped kernel binary and maps the source paths recorded in the debug
information to the working copy:

```sh
make qemu-debug
gdb
```

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
