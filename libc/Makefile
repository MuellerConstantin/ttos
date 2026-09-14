#!/usr/bin/env make

.PHONY: all clean

ROOTDIR ?= $(realpath ..)

# Warnings and anything else the subprojects share.
include $(ROOTDIR)/config.mk

CC := gcc
AR := ar

SRCDIR := src
OBJDIR := obj

TARGET := libc.a

INCLUDE := -I '$(ROOTDIR)/libc/include' -I '$(ROOTDIR)/libsys/include' -I '$(ROOTDIR)/abi/include'

# -MMD -MP records the headers each object was compiled against, so a header
# change recompiles what includes it; -MP keeps a deleted header from breaking
# the build.
CFLAGS := -c -std=c99 -ffreestanding -m32 $(WARNINGS) -MMD -MP
ARFLAGS := rcs

SRCS := $(shell find $(SRCDIR) -name '*.c')
OBJS := $(subst $(SRCDIR), $(OBJDIR), $(patsubst %.c, %.o, $(SRCS)))
DEPS := $(OBJS:.o=.d)

all: $(TARGET)

clean:

	rm -rf $(OBJDIR)
	rm -f $(TARGET)

$(TARGET): $(OBJS)

	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c

	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<

-include $(DEPS)
