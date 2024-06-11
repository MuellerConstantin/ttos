#!/usr/bin/env make

.PHONY: all clean

ARCH ?= x86
PLATFORM ?= intel
VERSION := 0.1.0
ROOTDIR := $(realpath .)

export ARCH
export PLATFORM
export ROOTDIR

QEMU := qemu-system-i386

ISODIR := iso

TARGET := kernel/kernel.elf
IMAGE := ttos-$(VERSION)-$(PLATFORM)-$(ARCH).iso

QEMUFLAGS := -cdrom $(IMAGE)

all: boot/grub/grub.cfg $(TARGET)

	mkdir -p $(ISODIR)/boot/grub

	cp boot/grub/grub.cfg $(ISODIR)/boot/grub
	cp $(TARGET) $(ISODIR)/boot

	grub-mkrescue --output=$(IMAGE) $(ISODIR)
	rm -rf $(ISODIR)

clean:

	$(MAKE) -C kernel clean

	rm -f $(IMAGE)

qemu:

	$(QEMU) $(QEMUFLAGS)

$(TARGET):

	$(MAKE) -C kernel all
