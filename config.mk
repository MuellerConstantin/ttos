#!/usr/bin/env make

# Configuration the build and the programs have to agree on. Everything here
# reaches C code as a macro, see the CFLAGS in userland/program.mk.

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
