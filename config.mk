#!/usr/bin/env make

# Configuration the build and the programs have to agree on. Everything here
# reaches C code as a macro, see the CFLAGS in userland/program.mk.

# Warnings every subproject is built with. EXTRA_WARNINGS is added to them for a
# single run without changing what the normal build reports, for example
# `make kernel EXTRA_WARNINGS=-Wconversion` to find assignments that silently
# truncate a value, which -Wall and -Wextra say nothing about.
WARNINGS ?= -Wall -Wextra $(EXTRA_WARNINGS)

# Labels the system volumes carry. The build writes them onto the file systems
# it creates, init finds the volume the system runs from by them, and an
# installer labels the file system it creates the same way. The live medium and
# an installed disk hold the same tree and are told apart by their label.
LIVE_LABEL ?= ttos-live
SYSTEM_LABEL ?= ttos-system

# First sector of the first partition, in 512 byte sectors. The gap in front of
# it holds the second stage of the bootloader, so it has to be large enough for
# that; 2048 is what partitioning tools have aligned to for a long time.
PARTITION_START ?= 2048

# Where core.img, the second stage of the bootloader, is written, in 512 byte
# sectors: into the gap between the master boot record and the first partition.
# Both the build and an installer put it there.
CORE_SECTOR ?= 1

# Bytes of boot.img that belong into the master boot record. Its code ends
# there; what follows is the disk signature, the partition table and the boot
# signature of the disk being written to, none of which may be overwritten.
# boot.img carries its floppy fallback in that space, which a hard disk has no
# use for.
MBR_CODE_SIZE ?= 440
