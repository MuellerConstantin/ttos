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

LOOPDEV := $(shell sudo losetup -f)

TARGET := kernel/kernel.elf
INITRD := initrd.img
HDA := hda.img
SDA := sda.img
IMAGE := ttos-$(VERSION)-$(PLATFORM)-$(ARCH).iso

# Userland binaries that must live on the initrd (bootstrap + rescue set).
# Everything else built into userland/bin is deployed to the disk images.
INITRD_BINS := init.elf lsvol.elf poweroff.elf lsdev.elf lsmnt.elf

# find-expression that excludes the initrd binaries when populating the disks,
# keeping the initrd set and the disk set a single source of truth.
INITRD_EXCLUDE := $(foreach bin,$(INITRD_BINS),! -name $(bin))

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

	rm -f initrd/*.elf
	for bin in $(INITRD_BINS); do cp $(ROOTDIR)/userland/bin/$$bin initrd/; done

	./scripts/mkinitrd.py -o $(INITRD) -i initrd

$(HDA):

	-while mountpoint -q mnt; do sudo umount mnt; done
	-sudo rm -rf mnt

	dd if=/dev/zero of=$(HDA) bs=1M count=50
	(echo n; echo p; echo 1; echo ; echo ; echo t; echo 83; echo w) | fdisk $(HDA)

	sudo losetup -P $(LOOPDEV) $(HDA)

	sudo mkfs.ext2 -b 1024 -I 128 -O ^resize_inode,^dir_index,^ext_attr,^metadata_csum,^64bit,^huge_file,^flex_bg $(LOOPDEV)p1

	mkdir -p mnt

	sudo mount $(LOOPDEV)p1 mnt

	sudo cp -r hdd/* mnt

	$(MAKE) -C $(ROOTDIR)/userland all
	sudo find $(ROOTDIR)/userland/bin -maxdepth 1 -type f -name '*.elf' $(INITRD_EXCLUDE) -exec cp {} mnt/bin/ \;

	sudo umount mnt

	sudo losetup -d $(LOOPDEV)

	rm -rf mnt

$(SDA):

	-while mountpoint -q mnt; do sudo umount mnt; done
	-sudo rm -rf mnt

	dd if=/dev/zero of=$(SDA) bs=1M count=50
	(echo n; echo p; echo 1; echo ; echo ; echo t; echo 83; echo w) | fdisk $(SDA)

	sudo losetup -P $(LOOPDEV) $(SDA)

	sudo mkfs.ext2 -b 1024 -I 128 -O ^resize_inode,^dir_index,^ext_attr,^metadata_csum,^64bit,^huge_file,^flex_bg $(LOOPDEV)p1

	mkdir -p mnt

	sudo mount $(LOOPDEV)p1 mnt

	sudo cp -r hdd/* mnt

	$(MAKE) -C $(ROOTDIR)/userland all
	sudo find $(ROOTDIR)/userland/bin -maxdepth 1 -type f -name '*.elf' $(INITRD_EXCLUDE) -exec cp {} mnt/bin/ \;

	sudo umount mnt

	sudo losetup -d $(LOOPDEV)

	rm -rf mnt
