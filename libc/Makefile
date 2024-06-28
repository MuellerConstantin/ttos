#!/usr/bin/env make

.PHONY: all clean

ROOTDIR ?= $(realpath ..)

CC := gcc
AR := ar

SRCDIR := src
OBJDIR := obj

LIBC_TARGET := libc.a
LIBK_TARGET := libk.a

INCLUDE := -I '$(ROOTDIR)/libc/include'

CFLAGS := -std=c99 -ffreestanding -m32 -Wall -Wextra
ARFLAGS := rcs
LIBC_CFLAGS := -D __is_libc
LIBK_CFLAGS := -D __is_libk

SRCS := $(shell find $(SRCDIR) -name '*.c')
LIBC_OBJS := $(subst $(SRCDIR), $(OBJDIR), $(patsubst %.c, %.libc.o, $(SRCS)))
LIBK_OBJS := $(subst $(SRCDIR), $(OBJDIR), $(patsubst %.c, %.libk.o, $(SRCS)))

all: $(LIBC_TARGET) $(LIBK_TARGET)

clean:

	rm -rf $(OBJDIR)
	rm -f $(LIBC_TARGET)
	rm -f $(LIBK_TARGET)

$(LIBC_TARGET): $(LIBC_OBJS)

	$(AR) $(ARFLAGS) $@ $^

$(LIBK_TARGET): $(LIBK_OBJS)

	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.libc.o: $(SRCDIR)/%.c

	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LIBC_CFLAGS) $(INCLUDE) -o $@ -c $<

$(OBJDIR)/%.libk.o: $(SRCDIR)/%.c

	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(LIBK_CFLAGS) $(INCLUDE) -o $@ -c $<
