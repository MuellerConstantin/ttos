#!/usr/bin/env make

.PHONY: all clean

ROOTDIR ?= $(realpath ..)

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
LIBC := $(ROOTDIR)/libc/libc.a
LIBSYS := $(ROOTDIR)/libsys/libsys.a
LIBK := $(ROOTDIR)/libk/libk.a

INCLUDE := -I '$(ROOTDIR)/kernel/include' -I '$(ROOTDIR)/libk/include' -I '$(ROOTDIR)/libsys/include' -I '$(ROOTDIR)/libc/include'

CFLAGS := -c -std=c99 -ffreestanding -m32 -Wall -Wextra -O -fno-stack-protector -g -D __KERNEL_VERSION__=\"$(VERSION)\" -D __KERNEL_ARCH__=\"$(ARCH)\" -D __KERNEL_PLATFORM__=\"$(PLATFORM)\"
ASFLAGS := -f elf32 -g
LDFLAGS := -m $(FORMAT) -T kernel.ld

SRCS := $(shell find $(SRCDIR) -name '*.asm') $(shell find $(SRCDIR) -name '*.c')
OBJS := $(subst $(SRCDIR), $(OBJDIR), $(patsubst %.c, %.o, $(patsubst %.asm, %.o, $(SRCS))))

all: $(TARGET)

clean:

	$(MAKE) -C $(ROOTDIR)/libc clean
	$(MAKE) -C $(ROOTDIR)/libsys clean
	$(MAKE) -C $(ROOTDIR)/libk clean

	rm -rf $(OBJDIR)
	rm -f $(TARGET)

$(TARGET): $(OBJS) $(LIBC) $(LIBSYS) $(LIBK)

	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBK) $(LIBSYS) $(LIBC)

$(LIBC):

	$(MAKE) -C $(ROOTDIR)/libc all

$(LIBSYS):

	$(MAKE) -C $(ROOTDIR)/libsys all

$(LIBK):

	$(MAKE) -C $(ROOTDIR)/libk all

$(OBJDIR)/%.o: $(SRCDIR)/%.asm

	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<
	
$(OBJDIR)/%.o: $(SRCDIR)/%.c

	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<
