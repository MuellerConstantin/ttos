#!/usr/bin/env make

.PHONY: all clean

ROOTDIR ?= $(realpath ..)

CC := gcc
AR := ar

SRCDIR := src
OBJDIR := obj

TARGET := libk.a

INCLUDE := -I '$(ROOTDIR)/libk/include' -I '$(ROOTDIR)/libc/include'

CFLAGS := -std=c99 -ffreestanding -m32 -Wall -Wextra
ARFLAGS := rcs

SRCS := $(shell find $(SRCDIR) -name '*.c')
OBJS := $(subst $(SRCDIR), $(OBJDIR), $(patsubst %.c, %.o, $(SRCS)))

all: $(TARGET)

clean:

	rm -rf $(OBJDIR)
	rm -f $(TARGET)

$(TARGET): $(OBJS)

	$(AR) $(ARFLAGS) $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c

	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ -c $<
