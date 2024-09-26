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
INITRD := initrd.img
HDD := hdd.img
IMAGE := ttos-$(VERSION)-$(PLATFORM)-$(ARCH).iso

QEMUFLAGS := -boot order=d -cdrom $(IMAGE) -display gtk,zoom-to-fit=on -vga std -m 4G -d int -no-reboot

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
	rm -rf $(HDD)
	rm -f $(IMAGE)

qemu: $(HDD)

	$(QEMU) $(QEMUFLAGS) -drive file=$(HDD),format=raw,index=0,if=ide

qemu-debug: $(HDD)

	$(QEMU) $(QEMUFLAGS) -drive file=$(HDD),format=raw,index=0,if=ide -s -S

$(TARGET):

	$(MAKE) -C kernel all

$(INITRD):

	./scripts/mkinitrd.py -o $(INITRD)

$(HDD):

	dd if=/dev/zero of=hdd.img bs=1M count=500
	echo "2,,83;" | sfdisk $(HDD)
