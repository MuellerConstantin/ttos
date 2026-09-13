#!/usr/bin/env make

# Shared rules for every userland program. A program's own Makefile sets
# TARGET and includes this file; its sources are expected under src/.

.PHONY: all clean

ROOTDIR ?= $(realpath ../..)

include $(ROOTDIR)/config.mk

LD := ld
CC := gcc

SRCDIR := src
OBJDIR := obj

FORMAT := elf_i386
LIBC := $(ROOTDIR)/libc/libc.a
LIBSYS := $(ROOTDIR)/libsys/libsys.a

INCLUDE := -I '$(ROOTDIR)/abi/include' -I '$(ROOTDIR)/libsys/include' -I '$(ROOTDIR)/libc/include'

CFLAGS := -c -std=c99 -ffreestanding -m32 -Wall -Wextra -O0 -fno-stack-protector -g -MMD -MP \
          -DTTOS_LIVE_LABEL='"$(LIVE_LABEL)"' -DTTOS_SYSTEM_LABEL='"$(SYSTEM_LABEL)"' \
          -DTTOS_PARTITION_START=$(PARTITION_START)
LDFLAGS := -m $(FORMAT) -e _start -nostdlib

SRCS := $(shell find $(SRCDIR) -name '*.c')
OBJS := $(subst $(SRCDIR), $(OBJDIR), $(patsubst %.c, %.o, $(SRCS)))
DEPS := $(OBJS:.o=.d)

all: $(TARGET)

clean:

	rm -rf $(OBJDIR)
	rm -f $(TARGET)

$(TARGET): $(OBJS) $(LIBSYS) $(LIBC)

	$(LD) $(LDFLAGS) -o $@ $(OBJS) --start-group $(LIBSYS) $(LIBC) --end-group

$(OBJDIR)/%.o: $(SRCDIR)/%.c

	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

-include $(DEPS)
