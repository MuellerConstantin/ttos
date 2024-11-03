#!/usr/bin/env make

.PHONY: all clean

ROOTDIR ?= $(realpath ..)
USERLANDDIR := $(ROOTDIR)/userland

DIRS := $(shell find $(USERLANDDIR) -mindepth 1 -maxdepth 1 -type d)

all:

	@for d in $(DIRS); do \
		$(MAKE) -C $$d all; \
	done

clean:

	@for d in $(DIRS); do \
		$(MAKE) -C $$d clean; \
	done
