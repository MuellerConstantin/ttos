#!/usr/bin/env make

.PHONY: all clean

ROOTDIR ?= $(realpath ../..)

LD := ld
CC := gcc
AS := nasm

SRCDIR := src
OBJDIR := obj

FORMAT := elf_i386
TARGET := init.elf
LIBC := $(ROOTDIR)/libc/libc.a

INCLUDE := -I '$(ROOTDIR)/libc/include'

CFLAGS := -c -std=c99 -ffreestanding -m32 -Wall -Wextra -O0 -fno-stack-protector -g
LDFLAGS := -m $(FORMAT) -e _start

SRCS := $(shell find $(SRCDIR) -name '*.asm') $(shell find $(SRCDIR) -name '*.c')
OBJS := $(subst $(SRCDIR), $(OBJDIR), $(patsubst %.c, %.o, $(patsubst %.asm, %.o, $(SRCS))))

all: $(TARGET)

clean:

	rm -rf $(OBJDIR)
	rm -f $(TARGET)

$(TARGET): $(OBJS)

	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBC)
	
$(OBJDIR)/%.o: $(SRCDIR)/%.c

	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<
