#!/usr/bin/env make

.PHONY: all clean

ROOTDIR ?= $(realpath ..)

# Warnings and anything else the subprojects share.
include $(ROOTDIR)/config.mk

ifndef VERSION
$(error Required variable VERSION is not set)
endif

ifndef ARCH
$(error Required variable ARCH is not set)
endif

ifndef PLATFORM
$(error Required variable PLATFORM is not set)
endif

LD := ld
CC := gcc
AS := nasm

SRCDIR := src
OBJDIR := obj

FORMAT := elf_i386
TARGET := kernel.elf

INCLUDE := -I '$(ROOTDIR)/kernel/include' -I '$(ROOTDIR)/abi/include'

# -MMD -MP records the headers each object was compiled against, so a header
# change recompiles what includes it; -MP keeps a deleted header from breaking
# the build.
CFLAGS := -c -std=c99 -ffreestanding -m32 $(WARNINGS) -O -fno-stack-protector -g -D __KERNEL_VERSION__=\"$(VERSION)\" -D __KERNEL_ARCH__=\"$(ARCH)\" -D __KERNEL_PLATFORM__=\"$(PLATFORM)\" -MMD -MP
ASFLAGS := -f elf32 -g
LDFLAGS := -m $(FORMAT) -T kernel.ld -nostdlib

SRCS := $(shell find $(SRCDIR) -name '*.asm') $(shell find $(SRCDIR) -name '*.c')
OBJS := $(subst $(SRCDIR), $(OBJDIR), $(patsubst %.c, %.o, $(patsubst %.asm, %.o, $(SRCS))))
DEPS := $(OBJS:.o=.d)

all: $(TARGET)

clean:

	$(MAKE) -C $(ROOTDIR)/libsys clean

	rm -rf $(OBJDIR)
	rm -f $(TARGET)

$(TARGET): $(OBJS)

	$(LD) $(LDFLAGS) -o $@ $(OBJS)

$(OBJDIR)/%.o: $(SRCDIR)/%.asm

	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<
	
$(OBJDIR)/%.o: $(SRCDIR)/%.c

	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

-include $(DEPS)
