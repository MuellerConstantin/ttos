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
HDA := hda.img
SDA := sda.img
IMAGE := ttos-$(VERSION)-$(PLATFORM)-$(ARCH).iso

QEMUFLAGS := -boot order=d -cdrom $(IMAGE) -display gtk,zoom-to-fit=on -vga std -m 2G -d int -no-reboot

all: boot/grub/grub.cfg $(INITRD) $(TARGET)

	mkdir -p $(ISODIR)/boot/grub

	cp boot/grub/grub.cfg $(ISODIR)/boot/grub
	cp $(TARGET) $(ISODIR)/boot
	cp $(INITRD) $(ISODIR)/boot

	grub-mkrescue --output=$(IMAGE) $(ISODIR)
	rm -rf $(ISODIR)

.PHONY: kernel
kernel: $(TARGET)

clean:

	$(MAKE) -C kernel clean

	rm -rf $(INITRD)
	rm -rf $(HDA)
	rm -rf $(SDA)
	rm -f $(IMAGE)

qemu: $(HDA) $(SDA)

	$(QEMU) $(QEMUFLAGS) -drive file=$(HDA),format=raw,index=0,if=ide,id=hda -drive file=$(SDA),format=raw,if=none,id=sda -device ahci,id=ahci -device ide-hd,drive=sda,bus=ahci.0

qemu-debug: $(HDA) $(SDA)

	$(QEMU) $(QEMUFLAGS) -drive file=$(HDA),format=raw,index=0,if=ide,id=hda -drive file=$(SDA),format=raw,if=none,id=sda -device ahci,id=ahci -device ide-hd,drive=sda,bus=ahci.0 -s -S

$(TARGET):

	$(MAKE) -C kernel all

$(INITRD):

	./scripts/mkinitrd.py -o $(INITRD) -i initrd

$(HDA):

	dd if=/dev/zero of=hda.img bs=1M count=500
	echo "2,,83;" | sfdisk $(HDA)

$(SDA):

	dd if=/dev/zero of=sda.img bs=1M count=500
	echo "2,,83;" | sfdisk $(SDA)
