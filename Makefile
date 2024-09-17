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
QEMU-IMG := qemu-img

ISODIR := iso

TARGET := kernel/kernel.elf
INITRD := initrd.img
IMAGE := ttos-$(VERSION)-$(PLATFORM)-$(ARCH).iso

QEMUFLAGS := -cdrom $(IMAGE) -display gtk,zoom-to-fit=on -vga std -m 4G

all: boot/grub/grub.cfg $(INITRD) $(TARGET)

	mkdir -p $(ISODIR)/boot/grub

	cp boot/grub/grub.cfg $(ISODIR)/boot/grub
	cp $(TARGET) $(ISODIR)/boot
	cp $(INITRD) $(ISODIR)/boot

	grub-mkrescue --output=$(IMAGE) $(ISODIR)
	rm -rf $(ISODIR)

clean:

	$(MAKE) -C kernel clean

	rm -rf $(INITRD)
	rm -f $(IMAGE)

qemu:

	$(QEMU-IMG) create -f raw hdd.img 500M
	$(QEMU) $(QEMUFLAGS) -drive file=hdd.img,format=raw,index=0,if=ide

$(TARGET):

	$(MAKE) -C kernel all

$(INITRD):

	./scripts/mkinitrd.py -o $(INITRD)
