#!/usr/bin/env make

.PHONY: all clean

VERSION := 0.1.0
ARCH ?= x86
PLATFORM ?= intel
ROOTDIR := $(realpath .)

export VERSION
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

	$(MAKE) -C $(ROOTDIR)/kernel clean

	$(MAKE) -C $(ROOTDIR)/userland clean
	rm -f initrd/*.elf

	rm -rf $(INITRD)
	rm -f $(IMAGE)

qemu:

	$(QEMU) $(QEMUFLAGS) -drive file=$(HDA),format=raw,index=0,if=ide,id=hda -drive file=$(SDA),format=raw,if=none,id=sda -device ahci,id=ahci -device ide-hd,drive=sda,bus=ahci.0

qemu-debug:

	$(QEMU) $(QEMUFLAGS) -drive file=$(HDA),format=raw,index=0,if=ide,id=hda -drive file=$(SDA),format=raw,if=none,id=sda -device ahci,id=ahci -device ide-hd,drive=sda,bus=ahci.0 -s -S

.PHONY: qemu-disk
qemu-disk: $(HDA) $(SDA)

qemu-clean:

	rm -f $(HDA)
	rm -f $(SDA)

$(TARGET):

	$(MAKE) -C kernel all

$(INITRD):

	$(MAKE) -C $(ROOTDIR)/userland all
	cp -r $(ROOTDIR)/userland/bin/* initrd

	./scripts/mkinitrd.py -o $(INITRD) -i initrd

$(HDA):

	dd if=/dev/zero of=$(HDA) bs=1M count=50
	(echo n; echo p; echo 1; echo ; echo ; echo t; echo b; echo w) | fdisk $(HDA)

	sudo losetup -P /dev/loop1 $(HDA)

	sudo mkfs.fat -F 32 /dev/loop1p1

	mkdir -p mnt

	sudo mount /dev/loop1p1 mnt

	sudo cp -r hdd/* mnt

	sudo umount mnt

	sudo losetup -d /dev/loop1

	rm -rf mnt

$(SDA):

	dd if=/dev/zero of=$(SDA) bs=1M count=50
	(echo n; echo p; echo 1; echo ; echo ; echo t; echo b; echo w) | fdisk $(SDA)

	sudo losetup -P /dev/loop1 $(SDA)

	sudo mkfs.fat -F 32 /dev/loop1p1

	mkdir -p mnt

	sudo mount /dev/loop1p1 mnt

	sudo cp -r hdd/* mnt

	sudo umount mnt

	sudo losetup -d /dev/loop1

	rm -rf mnt
