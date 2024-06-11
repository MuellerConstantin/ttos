#!/usr/bin/env make

.PHONY: all clean

FORMAT := elf_i386

TARGET := kernel.elf

LDFLAGS := -m $(FORMAT) -T kernel.ld

LIBS := \
	core/core.a \
	arch/$(ARCH)/arch.a \
	platform/$(PLATFORM)/platform.a

all: $(TARGET)

clean:

	$(MAKE) -C core clean
	$(MAKE) -C arch/$(ARCH) clean
	$(MAKE) -C platform/$(PLATFORM) clean

	rm -f $(TARGET)

$(TARGET): $(LIBS)

	$(LD) $(LDFLAGS) --start-group $^ --end-group -o $@

$(LIBS):

	$(MAKE) -C core all
	$(MAKE) -C arch/$(ARCH) all
	$(MAKE) -C platform/$(PLATFORM) all
