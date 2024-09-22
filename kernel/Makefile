#!/usr/bin/env make

.PHONY: all clean

ROOTDIR ?= $(realpath ..)

CC := gcc
AS := nasm

SRCDIR := src
OBJDIR := obj

FORMAT := elf_i386
TARGET := kernel.elf
LIBK := $(ROOTDIR)/libc/libk.a

INCLUDE := -I '$(ROOTDIR)/kernel/include' -I '$(ROOTDIR)/libc/include'

CFLAGS := -std=c99 -ffreestanding -m32 -Wall -Wextra -O -fno-stack-protector -g
ASFLAGS := -f elf -g

SRCS := $(shell find $(SRCDIR) -name '*.asm') $(shell find $(SRCDIR) -name '*.c')
OBJS := $(subst $(SRCDIR), $(OBJDIR), $(patsubst %.c, %.o, $(patsubst %.asm, %.o, $(SRCS))))

LDFLAGS := -m $(FORMAT) -T kernel.ld

all: $(TARGET)

clean:

	$(MAKE) -C $(ROOTDIR)/libc clean

	rm -rf $(OBJDIR)
	rm -f $(TARGET)

$(TARGET): $(OBJS) $(LIBK)

	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBK)

$(LIBK):

	$(MAKE) -C $(ROOTDIR)/libc all

$(OBJDIR)/%.o: $(SRCDIR)/%.asm

	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<
	
$(OBJDIR)/%.o: $(SRCDIR)/%.c

	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<
