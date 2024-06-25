#!/usr/bin/env make

.PHONY: all clean

CC := gcc
AS := nasm

SRCDIR := src
OBJDIR := obj

FORMAT := elf_i386
TARGET := kernel.elf

INCLUDE := -I '$(ROOTDIR)/kernel/include'

CFLAGS := -std=c99 -ffreestanding -m32 -Wall -O -fno-stack-protector
ASFLAGS := -f elf

SRCS := $(shell find $(SRCDIR) -name '*.asm') $(shell find $(SRCDIR) -name '*.c')
OBJS := $(subst $(SRCDIR), $(OBJDIR), $(patsubst %.c, %.o, $(patsubst %.asm, %.o, $(SRCS))))

LDFLAGS := -m $(FORMAT) -T kernel.ld

all: $(TARGET)

clean:

	rm -rf $(OBJDIR)
	rm -f $(TARGET)

$(TARGET): $(OBJS)

	$(LD) $(LDFLAGS) --start-group $^ --end-group -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.asm

	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<
	
$(OBJDIR)/%.o: $(SRCDIR)/%.c

	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<
