#!/usr/bin/env make

.PHONY: all clean

# A disk image whose build fails halfway through is worse than no image at all:
# it would still satisfy the target and get booted.
.DELETE_ON_ERROR:

VERSION := 0.1.0
ARCH ?= x86
PLATFORM ?= intel
ROOTDIR := $(realpath .)

include config.mk

export VERSION
export ARCH
export PLATFORM
export ROOTDIR

QEMU := qemu-system-i386

ISODIR := iso

TARGET := kernel/kernel.elf
INITRD := initrd.img
SYSTEMRD := systemrd.img
HDA := hda.img
SDA := sda.img
IMAGE := ttos-$(VERSION)-$(PLATFORM)-$(ARCH).iso

# The system tree as it ends up on a disk: the hdd skeleton, the userland that
# is not on the initrd, and /boot with kernel, initrd and the installed GRUB
# menu. It is packed into the system ramdisk for the live medium and copied
# onto the installed disk, so both carry the same files.
SYSROOT := sysroot

# Size of the system ramdisk. The tree is well below 1 MiB, the rest is room
# to work in while running live.
SYSTEMRD_SIZE := 4M

# Size of the generated disk images in MiB.
DISK_SIZE := 50

# Userland binaries that must live on the initrd.
INITRD_BINS := init.elf shell.elf clear.elf lsvol.elf poweroff.elf lsdev.elf lsmnt.elf lsproc.elf mount.elf unmount.elf dmesg.elf uptime.elf memusage.elf memmap.elf kheapusage.elf

# Userland binaries that must live on the disk.
INITRD_EXCLUDE := $(foreach bin,$(INITRD_BINS),! -name $(bin))

# ext2 feature set understood by the kernel's ext2 driver.
MKFS_FLAGS := -b 1024 -I 128 -O ^resize_inode,^dir_index,^ext_attr,^metadata_csum,^64bit,^huge_file,^flex_bg

# Where the host's GRUB keeps the boot images for the BIOS target.
GRUB_LIB ?= /usr/lib/grub/i386-pc
GRUB_MODULES := biosdisk part_msdos ext2 normal multiboot

# Where core.img looks for grub.cfg. It is compressed together with the image
# and cannot be patched afterwards, so the layout of an installed disk is
# fixed here, at build time.
GRUB_PREFIX := (hd0,msdos1)/boot/grub

# Size of the partition, derived from the disk and the gap in front of it. The
# block count assumes the 1 KiB blocks MKFS_FLAGS asks for.
PARTITION_SECTORS := $(shell expr $(DISK_SIZE) \* 2048 - $(PARTITION_START))
PARTITION_BLOCKS := $(shell expr $(PARTITION_SECTORS) / 2)
PARTITION_IMAGE := hda-part.img

# Renders one of the GRUB menus in boot/grub, stamping the entry with the
# version and the medium the system was started from, so it is visible on the
# GRUB screen whether the live medium or the installed disk was booted.
grub-cfg = sed 's|menuentry "TTOS"|menuentry "TTOS $(VERSION) ($(2))"|' boot/grub/$(1)

QEMU_COMMON := -display gtk,zoom-to-fit=on -vga std -m 2G -d int -no-reboot

QEMU_DISKS := -drive file=$(HDA),format=raw,index=0,if=ide,id=hda \
              -drive file=$(SDA),format=raw,if=none,id=sda \
              -device ahci,id=ahci -device ide-hd,drive=sda,bus=ahci.0

QEMU_GDB = $(if $(GDB),-s -S)

all: $(IMAGE)

# Creates a bootable live image. GRUB loads the system ramdisk as a second
# module; the kernel presents it as a RAM disk and the system runs from it.
$(IMAGE): boot/grub/live.cfg $(INITRD) $(SYSTEMRD) $(TARGET)

	mkdir -p $(ISODIR)/boot/grub

	$(call grub-cfg,live.cfg,Live) > $(ISODIR)/boot/grub/grub.cfg
	cp $(TARGET) $(ISODIR)/boot
	cp $(INITRD) $(ISODIR)/boot
	cp $(SYSTEMRD) $(ISODIR)/boot

	grub-mkrescue --output=$@ $(ISODIR)
	rm -rf $(ISODIR)

.PHONY: kernel
kernel: $(TARGET)

clean:

	$(MAKE) -C $(ROOTDIR)/kernel clean

	$(MAKE) -C $(ROOTDIR)/userland clean
	rm -f initrd/*.elf

	rm -rf $(INITRD)
	rm -rf $(SYSROOT)
	rm -f $(SYSTEMRD)
	rm -f $(IMAGE)

# Boot the live medium: GRUB, the kernel and both ramdisks come from the CD.
# hda is attached as the disk an installation would go onto.
.PHONY: qemu-live
qemu-live: $(IMAGE) $(HDA) $(SDA)

	$(QEMU) $(QEMU_COMMON) $(QEMU_GDB) -boot order=d -cdrom $(IMAGE) $(QEMU_DISKS)

# Boot the installed system: no CD at all, GRUB comes from hda's MBR and loads
# the kernel and initrd from hda's ext2 partition. hda is whatever the last
# installation left there - a live session's or hda-install's. A blank disk
# ends at the firmware's boot failure, as it would on a real machine.
.PHONY: qemu-installed
qemu-installed: $(HDA) $(SDA)

	$(QEMU) $(QEMU_COMMON) $(QEMU_GDB) -boot order=c $(QEMU_DISKS)

.PHONY: qemu
qemu: qemu-live

.PHONY: qemu-debug
qemu-debug: GDB := 1
qemu-debug: qemu-live

.PHONY: qemu-disk
qemu-disk: $(HDA) $(SDA)

.PHONY: qemu-clean
qemu-clean:

	rm -f $(HDA)
	rm -f $(SDA)

.PHONY: FORCE
FORCE:

$(TARGET): FORCE

	$(MAKE) -C kernel all

$(INITRD): FORCE

	$(MAKE) -C $(ROOTDIR)/userland all

	rm -f initrd/*.elf
	for bin in $(INITRD_BINS); do cp $(ROOTDIR)/userland/bin/$$bin initrd/; done

	./scripts/mkinitrd.py -o $(INITRD) -i initrd

# The system ramdisk: the system tree in an ext2 image, built without
# privileges. mkfs.ext2 populates the image from the tree directly.
$(SYSTEMRD): $(TARGET) $(INITRD) boot/grub/installed.cfg

	rm -rf $(SYSROOT)
	cp -r hdd $(SYSROOT)

	find $(ROOTDIR)/userland/bin -maxdepth 1 -type f -name '*.elf' $(INITRD_EXCLUDE) -exec cp {} $(SYSROOT)/bin/ \;

	cp $(TARGET) $(SYSROOT)/boot/kernel.elf
	cp $(INITRD) $(SYSROOT)/boot/initrd.img

	mkdir -p $(SYSROOT)/boot/grub/i386-pc
	$(call grub-cfg,installed.cfg,Installed) > $(SYSROOT)/boot/grub/grub.cfg

	cp $(GRUB_LIB)/boot.img $(SYSROOT)/boot/grub/i386-pc/boot.img
	grub-mkimage -O i386-pc -p '$(GRUB_PREFIX)' -o $(SYSROOT)/boot/grub/i386-pc/core.img $(GRUB_MODULES)

	rm -f $@
	mkfs.ext2 -q $(MKFS_FLAGS) -L $(LIVE_LABEL) -d $(SYSROOT) $@ $(SYSTEMRD_SIZE)

# A blank disk attached to the IDE controller.
$(HDA):

	dd if=/dev/zero of=$(HDA) bs=1M count=$(DISK_SIZE)

# A blank disk attached to the AHCI controller.
$(SDA):

	dd if=/dev/zero of=$(SDA) bs=1M count=$(DISK_SIZE)

# Install the system tree onto hda from the host: partition the disk, put the
# tree on its file system and the boot code in front of it. This is the
# reference for what an installed disk has to look like, and it writes the same
# bytes an installer running inside the system would have to write.
.PHONY: hda-install
hda-install: $(SYSTEMRD)

	dd if=/dev/zero of=$(HDA) bs=1M count=$(DISK_SIZE)
	(echo n; echo p; echo 1; echo $(PARTITION_START); echo ; echo t; echo 83; echo a; echo w) | fdisk $(HDA)

	mkfs.ext2 -q $(MKFS_FLAGS) -L $(SYSTEM_LABEL) -d $(SYSROOT) $(PARTITION_IMAGE) $(PARTITION_BLOCKS)
	dd if=$(PARTITION_IMAGE) of=$(HDA) bs=512 seek=$(PARTITION_START) conv=notrunc
	rm -f $(PARTITION_IMAGE)

	dd if=$(SYSROOT)/boot/grub/i386-pc/boot.img of=$(HDA) bs=1 count=$(MBR_CODE_SIZE) conv=notrunc
	dd if=$(SYSROOT)/boot/grub/i386-pc/core.img of=$(HDA) bs=512 seek=$(CORE_SECTOR) conv=notrunc
