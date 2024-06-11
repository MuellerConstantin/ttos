#!/usr/bin/env make

.PHONY: all clean

CC := gcc
AR := ar
AS := nasm

SRCDIR := src
OBJDIR := obj

TARGET := core.a

INCLUDE := \
	-I '$(ROOTDIR)/kernel/core/include' \
	-I '$(ROOTDIR)/kernel/arch/$(ARCH)/include' \
	-I '$(ROOTDIR)/kernel/platform/$(PLATFORM)/include'

CFLAGS := -std=c99 -ffreestanding -m32 -Wall -O -fno-stack-protector
ASFLAGS := -f elf
ARFLAGS := -rcs

SRCS := $(shell find $(SRCDIR) -name '*.asm') $(shell find $(SRCDIR) -name '*.c')
OBJS := $(subst $(SRCDIR), $(OBJDIR), $(patsubst %.c, %.o, $(patsubst %.asm, %.o, $(SRCS))))

all: $(TARGET)

clean:

	rm -rf $(OBJDIR)
	rm -f $(TARGET)

$(TARGET): $(OBJS)

	$(AR) $(ARFLAGS) $@ $^
	
$(OBJDIR)/%.o: $(SRCDIR)/%.asm

	mkdir -p $(@D)
	$(AS) $(ASFLAGS) -o $@ $<
	
$(OBJDIR)/%.o: $(SRCDIR)/%.c

	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<
