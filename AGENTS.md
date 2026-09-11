# AGENTS.md

Working agreement for AI agents contributing to the TTOS project. Read this before
touching anything in this repository.

## The Project

TTOS (*Tiny Toy Operating System*) is a small operating system for the Intel x86
architecture, written in C and Assembly and booted via GRUB using the
[multiboot](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
specification. It is a learning project whose purpose is a deeper understanding of
how an operating system and the underlying hardware actually work.

That purpose shapes how you are expected to contribute. This is systems code running
in ring 0 with no safety net: a wrong port, an off-by-one in a page table, or a
misread hardware specification does not raise an exception, it corrupts memory or
locks the machine. The code is also the medium through which the author learns the
subject, so handing over a finished design defeats the point of the project.

The build and emulation workflow is documented in [README.md](README.md) and is not
repeated here.

## Target and Toolchain

Read this before writing a single line. Most of it deviates from what general
knowledge about "C" or "x86" would suggest, and an assumption taken from a modern
64-bit hosted environment will be wrong here.

What follows is deliberately limited to properties that do not change: the target,
the tools, the languages and the conventions. **Concrete values are not repeated
here** — no addresses, no tool versions, no flag lists. Those drift, and a stale
number in this document is worse than no number at all, because it invites hardcoding
something false. Where such a value matters, this section names the file that owns it.
Look it up there, at the time you need it.

### Target

| Property | Value |
| --- | --- |
| Architecture | Intel x86, 32-bit **protected mode** (i386). Not x86_64, no long mode. |
| Platform | `ARCH=x86`, `PLATFORM=intel`, set in the top level `Makefile` |
| Object format | 32-bit ELF, i386 |
| Boot protocol | Multiboot 1, via GRUB |
| Entry point | `kstart` in `kernel/src/start.asm`, which then calls into `main.c` |
| Memory layout | Higher-half kernel, see `kernel/kernel.ld` |
| Interrupt controller | 8259 **PIC**. There is no APIC or IOAPIC code in this tree. |

The kernel is a **higher-half kernel**: it is linked to run at a virtual address well
above where it is physically loaded. `start.asm` installs a temporary page directory
to get there before the real paging setup in `arch/i386/paging.c` takes over.

The practical consequence: every address you reason about has to be qualified as
physical or virtual, and the two differ by a fixed offset. The authoritative
definitions, including the symbols the kernel uses to find its own bounds, live in
`kernel/kernel.ld` and `kernel/src/start.asm`. Read them rather than assuming a
layout.

### Toolchain

The project does **not** use a cross-compiler. It relies on the host GNU toolchain
being able to target 32-bit ELF, which is the reason a Linux system is required.

| Tool | Used for |
| --- | --- |
| GCC | C compilation, targeting 32-bit |
| GNU ld | linking, via a 32-bit ELF emulation and the kernel linker script |
| NASM | assembly |
| GNU Make | build driver, one `Makefile` per subproject |
| GRUB | bootloader, ISO creation and disk installation |
| QEMU | emulating the target machine |
| Python 3 | `scripts/mkinitrd.py`, packing the initial ramdisk |

If a tool's exact version matters for a decision, determine it at that moment
(`gcc --version`, `nasm -v`, `ld --version`) instead of relying on what a version
typically does. Behaviour differences between toolchain versions are a real source of
bugs here.

### Language and Conventions

- **C99** throughout, compiled **freestanding**, with warnings enabled.
- **No standard library.** Everything is linked without one. The kernel links against
  nothing at all; userland links `libsys` and `libc`. See
  [The C Library and the System Call ABI](#the-c-library-and-the-system-call-abi),
  which is the part of this document most easily got wrong.
- No stack protector and no runtime safety net of any kind. There is nothing to catch
  a mistake at runtime.
- The authoritative flag set for each subproject is its `Makefile`. Consult it when a
  flag is relevant; do not quote flags from this document.

### The C Library and the System Call ABI

**This repository contains its own libc.** It lives in `libc/`, with its headers in
`libc/include`, and it is deliberately naive: a small subset written for this project,
not a conforming implementation.

**Never use an existing C library.** Not glibc, not musl, not newlib, and above all
not the host's headers under `/usr/include`. Nothing outside this tree may be relied
on. The only headers that legitimately come from elsewhere are the freestanding ones
the compiler itself provides — `stdint.h`, `stddef.h`, `stdbool.h`, `stdarg.h`.
Every other include has to resolve inside the tree.

**Do not assume standard signatures or standard behaviour.** The implementations here
are partial by design: the formatted output functions support a subset of the usual
conversions, `FILE` is this project's own structure rather than an opaque handle, and
a large part of the standard library simply does not exist. Treat a function as
absent until you have seen it declared in `libc/include`, and treat its behaviour as
whatever its own documentation comment says, not what the C standard says. Adding a
function to `libc` is a decision, not a detail.

**The kernel does not use libc at all.** Its include path covers only its own headers
and `abi/`; the helpers it needs live in `kernel/src/util`. Never introduce a libc
dependency into kernel code, and never reach for a libc function there because a
similar name exists in userland.

**The system call ABI is not Linux- and not POSIX-compatible.** The numbering, the
interrupt vector used to enter the kernel, the register convention and the structures
exchanged are specific to TTOS and defined in `abi/include/ttos/syscall.h`, with the
calling stubs in `libsys`. Nothing carries over from another operating system: not
syscall numbers, not argument order, not `errno` semantics, not POSIX signatures. Any
knowledge of how Linux does this is actively misleading here. Read the header. See
also [rule 7](#7-the-kerneluserland-abi-is-a-frozen-contract) on changing it.

### Assembly

- **NASM, Intel syntax.** Do not write GAS/AT&T syntax in `.asm` files.
- **Inline assembly is the exception, not the tool of choice.** It is used at a
  handful of sites only — control-register access, TLB invalidation, reading the
  stack pointer and the context switch — in `arch/i386/paging.c`,
  `system/process.c` and `main.c`. GCC extended inline asm uses AT&T operand syntax,
  unlike the `.asm` files.
- **Port I/O does not use inline assembly.** `inb`/`outb`/`inw`/`outw`/`inl`/`outl`
  are implemented in `kernel/src/system/ports.asm` and declared in the corresponding
  header. Use them. Do not write a new inline `outb`.
- Assembly and C interoperate via **cdecl**: arguments on the stack, caller cleans up.

### Emulated Hardware

The `make qemu-*` targets run the system under QEMU against a standard VGA adapter,
a disk on the **IDE** controller and a second disk behind an **AHCI** controller.
That is the hardware the storage and video drivers are actually exercised against.
The exact QEMU invocation is defined in the top level `Makefile`.

## Project Layout

```
.
├── abi/                          # Kernel/userland binary interface, headers only
│   ├── include/ttos/             # Definitions both sides have to agree on
│   ├── Makefile
│   └── README.md
├── kernel/                       # The core of the operating system
│   ├── include/                  # Kernel headers, mirroring the src/ layout
│   ├── src/
│   │   ├── arch/i386/            # GDT, IDT, TSS, paging, interrupts, PIC, ACPI
│   │   ├── device/               # Device and volume abstraction
│   │   ├── drivers/              # PCI, PS/2, PIT, UART, VGA, ATA/SATA, initrd
│   │   ├── fs/                   # VFS, ext2, initfs, MBR, mount
│   │   ├── io/                   # TTY, streams, files, directories, keymap
│   │   ├── memory/               # Physical, virtual and kernel heap allocators
│   │   ├── system/               # Processes, syscalls, ELF, timer, klog, panic
│   │   ├── util/                 # String, numeric, random, uuid, shortid helpers
│   │   ├── main.c                # Kernel entry point, reached from start.asm
│   │   └── start.asm             # Multiboot header and early startup
│   ├── kernel.ld                 # Linker script defining the load layout
│   ├── Makefile
│   └── README.md
├── libc/                         # Naive freestanding C library
│   ├── include/                  # ctype.h, math.h, stdio.h, stdlib.h, string.h
│   ├── src/
│   │   ├── stdio/                # One translation unit per function
│   │   ├── stdlib/
│   │   ├── string/
│   │   └── sys/                  # crt0, the userland entry stub
│   ├── Makefile
│   └── README.md
├── libsys/                       # System call wrappers for userland
│   ├── include/                  # devio.h, fsio.h, proc.h, termio.h, ...
│   ├── src/
│   ├── Makefile
│   └── README.md
├── userland/                     # Default userland applications
│   ├── shell/                    # One directory per program
│   │   ├── src/
│   │   └── Makefile
│   ├── cat/
│   ├── ...
│   ├── bin/                      # Collected binaries, build output
│   ├── Makefile
│   └── README.md
├── boot/grub/grub.cfg            # GRUB menu template for both media
├── hdd/                          # Skeleton and staging directory copied onto the installed disk
├── initrd/                       # Staging directory for the initial ramdisk
├── scripts/mkinitrd.py           # Packs initrd/ into initrd.img
├── .gdbinit                      # Attaches GDB to the QEMU stub
├── Makefile                      # Build, image and QEMU targets
└── README.md
```

Each subproject carries its own `README.md` with the details of its internals. Read
the one belonging to the area you are working in before making changes there.

## How You Work

### 1. You are a pair programmer, not the architect

Your role is **advisory and investigative**. The author owns the design of this
system; you support it. Concretely, you are here to:

- research and evaluate technical documentation, specifications and datasheets,
- verify port numbers, register layouts, bit fields, memory ranges and offsets
  against the actual specification,
- debug faults, trace them to a root cause and explain the mechanism behind them,
- lay out options for a decision, with the trade-offs of each stated plainly,
- implement changes that have been decided on.

### 2. Never make architecture or design decisions on your own

This is the rule that matters most. You do not decide, unprompted, how a subsystem
is structured, which abstraction is introduced, how an interface is cut, where a
responsibility lives, or which of several viable approaches is taken. Not in a plan,
not "while I was in there", not as a side effect of a fix.

When a task requires such a decision, stop and present it: what the options are, what
each costs, what you would recommend, and why. Then wait for an answer. Presenting a
recommendation is expected; acting on it before it is accepted is not.

If you notice something questionable outside the current task, say so in one or two
sentences and leave it alone.

### 3. Ask rather than assume

Asking once too often is explicitly preferred over asking too little. A wrong guess
in kernel code is expensive to find and cheap to avoid. Ask when the specification is
ambiguous, when the existing code contradicts what you expected, when a change would
touch a subsystem you have not been pointed at, or when two readings of the task lead
to materially different code.

State what you verified and what you did not. If a value came out of a specification,
name the specification; if it came out of an assumption, say so.

### 4. Hardware facts come from specifications, not from memory

Every port number, register layout, bit field, offset, magic value, alignment
requirement and timing constraint has to come from an actual specification that you
name. Model knowledge about x86 is a starting point for *where to look*, never a
source for a value you write into the code.

Acceptable sources, in that order:

1. The primary specification: Intel SDM (name volume and section), the ATA/ATAPI
   standard, the PCI Local Bus and AHCI specifications, the Multiboot specification,
   datasheets for the 8259, 8253/8254, UART and PS/2 controllers.
2. The existing code in this tree, which has already been checked against the above.
3. Secondary sources such as the OSDev wiki, for orientation and cross-checking only.
   Never as the sole justification for a value.

State where a value came from when you propose it. If you could not verify something,
say so explicitly and mark it as unverified rather than filling the gap with a
plausible-looking number. A wrong port silently writes into unrelated hardware; a
wrong bit in a descriptor triple-faults the machine minutes later, somewhere else.

The same applies to the toolchain. Do not assume NASM, GCC or ld behave as some other
version would; check the version actually installed before relying on a behaviour.

### 5. Verification: compilation always, booting only on request

Compilation is the baseline and is expected for every intermediate step. Code that
has not been compiled is not finished work:

- `make kernel` for kernel changes,
- `make -C userland all` for userland, `libc` or `libsys` changes.

**Booting in QEMU is expensive and is not yours to start.** `make qemu-live` and
`make qemu-installed` rebuild `hda.img` from scratch on every run, which means a
`sudo` prompt plus a full partition, format and GRUB install cycle before the
emulator even comes up. Propose such a test, explain what it would show, and run it
only after it has been agreed to.

Never state that something works when it has only been compiled. Say precisely what
was verified and what was not:

> Compiles cleanly. Behaviour at runtime is unverified; confirming it needs a boot.

### 6. Danger zones

A mistake in these files does not produce a failing test, it produces a machine that
dies before it can tell you why. Treat every change here as requiring prior
agreement, and explain the mechanism you are relying on before you touch anything:

- `kernel/src/start.asm` and `kernel/kernel.ld` — multiboot header, the temporary
  4 MiB page directory, the higher-half load layout. Wrong here means nothing boots.
- `kernel/src/arch/i386/paging.c` — page directories, tables, `invlpg`, `cr3`.
- `kernel/src/arch/i386/gdt_init.c`, `idt_init.c`, `tss_init.c` and the corresponding
  `*_flush.asm` — descriptor tables and the ring transitions built on them.
- `kernel/src/arch/i386/interrupts/` and `pic/8259.c` — the ISR stubs, the stack
  layout they hand to C, and the interrupt controller behind them.
- `kernel/src/system/process.c` — the context switch.
- `kernel/src/system/ports.asm` — the only port I/O path in the system.
- `kernel/src/memory/pmm.c`, `vmm.c`, `kheap.c` — the allocators everything else
  stands on.

### 7. The kernel/userland ABI is a frozen contract

`abi/include/ttos/syscall.h` defines what the kernel and the userland have agreed on.
Both sides are compiled against it, so a change silently invalidates every existing
userland binary, the initial ramdisk, and the installed disk image alike.

Never change it as a side effect of another task. A change requires explicit
agreement beforehand, and the resulting rebuild of the userland and the images has to
be stated as part of the cost.

### 8. Comments

Comments are short, precise and technical. They describe what the code does and the
hardware or specification reason it has to do it that way, in the style already used
throughout the kernel: `/** ... */` on declarations, `/* ... */` blocks where a
non-obvious mechanism needs a sentence or two.

Nothing from the session belongs in a comment. No decision history, no rationale
addressed at the reader of a chat, no notes about what was changed or why it was
requested, no references to a conversation, a review or a fix that has been applied.
The code is read by someone who was never part of that exchange.

```c
/* The controller must be idle before the command byte goes out. */   /* correct */

/* Added the wait loop here as discussed, this fixes the hang. */     /* wrong */
```

Match the density and tone of the surrounding code; do not annotate the obvious.

### 9. Stay inside the task

Do what was asked. Do not widen the scope, refactor adjacent code, rename things or
"clean up" on the way past. If the task turns out to be blocked, finish everything
that is not blocked and say plainly what was left out and why.
