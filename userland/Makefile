#!/usr/bin/env make

.PHONY: all clean

ROOTDIR ?= $(realpath ..)
USERLANDDIR := $(ROOTDIR)/userland

DIRS := $(shell find $(USERLANDDIR) -mindepth 1 -maxdepth 1 -type d)

all:

	$(MAKE) -C $(ROOTDIR)/libc all
	$(MAKE) -C $(ROOTDIR)/libsys all

	mkdir -p $(ROOTDIR)/userland/bin

	@for d in $(DIRS); do \
		$(MAKE) -C $$d all; \
		cp $$d/$$(basename $$d).elf $(ROOTDIR)/userland/bin/$$(basename $$d).elf; \
	done

clean:

	$(MAKE) -C $(ROOTDIR)/libc clean
	$(MAKE) -C $(ROOTDIR)/libsys clean

	rm -rf $(ROOTDIR)/userland/bin

	@for d in $(DIRS); do \
		$(MAKE) -C $$d clean; \
	done
