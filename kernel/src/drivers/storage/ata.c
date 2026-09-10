#include <drivers/storage/ata.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <system/kmessage.h>
#include <device/device.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/types.h>

static ata_device_t ata_devices[4] = {
    {ATA_PRIMARY_MASTER_DRIVE, 0, false, false, false},
    {ATA_PRIMARY_SLAVE_DRIVE, 0, false, false, false},
    {ATA_SECONDARY_MASTER_DRIVE, 0, false, false, false},
    {ATA_SECONDARY_SLAVE_DRIVE, 0, false, false, false}
};

static device_t* ata_claim_controller(void);
static uint16_t ata_get_io_base(ata_device_t* device);
static bool ata_is_master(ata_device_t* device);
static bool ata_device_probe(ata_device_t* device);
static void ata_wait_busy(uint16_t io_base);
static bool ata_wait_data(uint16_t io_base);
static void ata_select_sector_lba28(uint16_t io_base, uint32_t lba);
static void ata_write_sector_lba28(ata_device_t* device, uint32_t lba, uint8_t* buffer);
static void ata_read_sector_lba28(ata_device_t* device, uint32_t lba, uint8_t* buffer);

static size_t ata_total_size_primary_master();
static size_t ata_write_primary_master(size_t offset, size_t size, char* buffer);
static size_t ata_read_primary_master(size_t offset, size_t size, char* buffer);
static size_t ata_total_size_primary_slave();
static size_t ata_write_primary_slave(size_t offset, size_t size, char* buffer);
static size_t ata_read_primary_slave(size_t offset, size_t size, char* buffer);
static size_t ata_total_size_secondary_master();
static size_t ata_write_secondary_master(size_t offset, size_t size, char* buffer);
static size_t ata_read_secondary_master(size_t offset, size_t size, char* buffer);
static size_t ata_total_size_secondary_slave();
static size_t ata_write_secondary_slave(size_t offset, size_t size, char* buffer);
static size_t ata_read_secondary_slave(size_t offset, size_t size, char* buffer);
static size_t ata_sector_size();

int32_t ata_init() {
    device_t* controller = ata_claim_controller();

    if(ata_device_probe(&ata_devices[ATA_PRIMARY_MASTER_DRIVE])) {
        storage_device_t *device = (storage_device_t*) kmalloc(sizeof(storage_device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->name = (char*) kmalloc(25);

        if(!device->name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device_generate_id(device->id);
        strcpy(device->name, "ATA Primary Master Drive");
        device->type = DEVICE_TYPE_STORAGE;
        device->bus.type = DEVICE_BUS_TYPE_ATA;
        device->bus.data = &ata_devices[ATA_PRIMARY_MASTER_DRIVE];

        device->driver.storage = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

        if(!device->driver.storage) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->driver.storage->sector_size = ata_sector_size;
        device->driver.storage->total_size = ata_total_size_primary_master;
        device->driver.storage->read = ata_read_primary_master;
        device->driver.storage->write = ata_write_primary_master;

        device_register(controller, device);
    }

    if(ata_device_probe(&ata_devices[ATA_PRIMARY_SLAVE_DRIVE])) {
        storage_device_t *device = (storage_device_t*) kmalloc(sizeof(storage_device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->name = (char*) kmalloc(24);

        if(!device->name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device_generate_id(device->id);
        strcpy(device->name, "ATA Primary Slave Drive");
        device->type = DEVICE_TYPE_STORAGE;
        device->bus.type = DEVICE_BUS_TYPE_ATA;
        device->bus.data = &ata_devices[ATA_PRIMARY_SLAVE_DRIVE];

        device->driver.storage = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

        if(!device->driver.storage) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->driver.storage->sector_size = ata_sector_size;
        device->driver.storage->total_size = ata_total_size_primary_slave;
        device->driver.storage->read = ata_read_primary_slave;
        device->driver.storage->write = ata_write_primary_slave;

        device_register(controller, device);
    }

    if(ata_device_probe(&ata_devices[ATA_SECONDARY_MASTER_DRIVE])) {
        storage_device_t *device = (storage_device_t*) kmalloc(sizeof(storage_device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->name = (char*) kmalloc(27);

        if(!device->name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device_generate_id(device->id);
        strcpy(device->name, "ATA Secondary Master Drive");
        device->type = DEVICE_TYPE_STORAGE;
        device->bus.type = DEVICE_BUS_TYPE_ATA;
        device->bus.data = &ata_devices[ATA_SECONDARY_MASTER_DRIVE];

        device->driver.storage = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

        if(!device->driver.storage) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->driver.storage->sector_size = ata_sector_size;
        device->driver.storage->total_size = ata_total_size_secondary_master;
        device->driver.storage->read = ata_read_secondary_master;
        device->driver.storage->write = ata_write_secondary_master;

        device_register(controller, device);
    }

    if(ata_device_probe(&ata_devices[ATA_SECONDARY_SLAVE_DRIVE])) {
        storage_device_t *device = (storage_device_t*) kmalloc(sizeof(storage_device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->name = (char*) kmalloc(26);

        if(!device->name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device_generate_id(device->id);
        strcpy(device->name, "ATA Secondary Slave Drive");
        device->type = DEVICE_TYPE_STORAGE;
        device->bus.type = DEVICE_BUS_TYPE_ATA;
        device->bus.data = &ata_devices[ATA_SECONDARY_SLAVE_DRIVE];

        device->driver.storage = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

        if(!device->driver.storage) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->driver.storage->sector_size = ata_sector_size;
        device->driver.storage->total_size = ata_total_size_secondary_slave;
        device->driver.storage->read = ata_read_secondary_slave;
        device->driver.storage->write = ata_write_secondary_slave;

        device_register(controller, device);
    }

    return 0;
}

static size_t ata_total_size_primary_master() {
    return ata_devices[ATA_PRIMARY_MASTER_DRIVE].size;
}

static size_t ata_write_primary_master(size_t offset, size_t size, char* buffer) {
    return ata_write(&ata_devices[ATA_PRIMARY_MASTER_DRIVE], offset, size, buffer);
}

static size_t ata_read_primary_master(size_t offset, size_t size, char* buffer) {
    return ata_read(&ata_devices[ATA_PRIMARY_MASTER_DRIVE], offset, size, buffer);
}

static size_t ata_total_size_primary_slave() {
    return ata_devices[ATA_PRIMARY_SLAVE_DRIVE].size;
}

static size_t ata_write_primary_slave(size_t offset, size_t size, char* buffer) {
    return ata_write(&ata_devices[ATA_PRIMARY_SLAVE_DRIVE], offset, size, buffer);
}

static size_t ata_read_primary_slave(size_t offset, size_t size, char* buffer) {
    return ata_read(&ata_devices[ATA_PRIMARY_SLAVE_DRIVE], offset, size, buffer);
}

static size_t ata_total_size_secondary_master() {
    return ata_devices[ATA_SECONDARY_MASTER_DRIVE].size;
}

static size_t ata_write_secondary_master(size_t offset, size_t size, char* buffer) {
    return ata_write(&ata_devices[ATA_SECONDARY_MASTER_DRIVE], offset, size, buffer);
}

static size_t ata_read_secondary_master(size_t offset, size_t size, char* buffer) {
    return ata_read(&ata_devices[ATA_SECONDARY_MASTER_DRIVE], offset, size, buffer);
}

static size_t ata_total_size_secondary_slave() {
    return ata_devices[ATA_SECONDARY_SLAVE_DRIVE].size;
}

static size_t ata_write_secondary_slave(size_t offset, size_t size, char* buffer) {
    return ata_write(&ata_devices[ATA_SECONDARY_SLAVE_DRIVE], offset, size, buffer);
}

static size_t ata_read_secondary_slave(size_t offset, size_t size, char* buffer) {
    return ata_read(&ata_devices[ATA_SECONDARY_SLAVE_DRIVE], offset, size, buffer);
}

static size_t ata_sector_size() {
    return ATA_SECTOR_SIZE;
}

static device_t* ata_claim_controller(void) {
    device_t* controller = pci_find_device(PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_IDE_CONTROLLER);

    if(!controller) {
        return NULL;
    }

    char* name = (char*) kmalloc(15);

    if(!name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strcpy(name, "IDE Controller");

    kfree(controller->name);

    controller->name = name;
    controller->type = DEVICE_TYPE_CONTROLLER;

    return controller;
}

static uint16_t ata_get_io_base(ata_device_t* device) {
    switch(device->drive) {
        case ATA_PRIMARY_MASTER_DRIVE:
        case ATA_PRIMARY_SLAVE_DRIVE:
            return ATA_PRIMARY_IO_BASE;
        case ATA_SECONDARY_MASTER_DRIVE:
        case ATA_SECONDARY_SLAVE_DRIVE:
            return ATA_SECONDARY_IO_BASE;
        default:
            return 0;
    }
}

static bool ata_is_master(ata_device_t* device) {
    switch(device->drive) {
        case ATA_PRIMARY_MASTER_DRIVE:
        case ATA_SECONDARY_MASTER_DRIVE:
            return true;
        case ATA_PRIMARY_SLAVE_DRIVE:
        case ATA_SECONDARY_SLAVE_DRIVE:
            return false;
        default:
            return false;
    }
}

static bool ata_device_probe(ata_device_t* device) {
    uint16_t io_base = ata_get_io_base(device);
    bool is_master = ata_is_master(device);

    // Choose master/slave drive
    outb(io_base + ATA_DRIVE_REGISTER, is_master ? 0xA0 : 0xB0);

    // Set the sector count and LBA registers to 0
    outb(io_base + ATA_SECTOR_COUNT_REGISTER, 0x00);
    outb(io_base + ATA_LBA_LOW_REGISTER, 0x00);
    outb(io_base + ATA_LBA_MID_REGISTER, 0x00);
    outb(io_base + ATA_LBA_HIGH_REGISTER, 0x00);

    // Send the identify command
    outb(io_base + ATA_COMMAND_REGISTER, 0xEC);

    if(inb(io_base + ATA_STATUS_REGISTER) == 0x00) {
        return false;
    }

    // Wait for the drive to be ready
    while(inb(io_base + ATA_STATUS_REGISTER) & 0x80);

    // Check if the drive is an ATA drive
    if(inb(io_base + ATA_LBA_MID_REGISTER) != 0x00 || inb(io_base + ATA_LBA_HIGH_REGISTER) != 0x00) {
        return false;
    }

    uint8_t drq = 0;
    uint8_t err = 0;

    // Wait for the drive to be ready
    while(!drq && !err) {
        drq = inb(io_base + ATA_STATUS_REGISTER) & 0x08;
        err = inb(io_base + ATA_STATUS_REGISTER) & 0x01;
    }

    if(err) {
        return false;
    }

    uint16_t identify_data[256];

    // Read the drive's identification data
    for(uint16_t i = 0; i < 256; i++) {
        identify_data[i] = inw(io_base + ATA_DATA_REGISTER);
    }

    // Check if LBA is supported
    if(identify_data[49] & (1 << 9)) {
        device->lba_supported = true;
    }

    /*
     * Check if LBA48 is supported. Bits 15:14 of word 83 must read 01 for the
     * word to carry valid data at all, otherwise the remaining bits are
     * whatever the drive happened to leave in there.
     */
    if((identify_data[83] & 0xC000) == 0x4000 && identify_data[83] & (1 << 10)) {
        device->lba48_supported = true;
    }

    uint64_t total_sectors = 0;

    // Get the drive's size
    if(device->lba_supported) {
        if(device->lba48_supported) {
            total_sectors = ((uint64_t) identify_data[100])
                          | ((uint64_t) identify_data[101] << 16)
                          | ((uint64_t) identify_data[102] << 32)
                          | ((uint64_t) identify_data[103] << 48);
        }

        /*
         * Words 60 and 61 hold the LBA28 capacity. They are the source for
         * drives without LBA48 and the fallback for drives that advertise
         * LBA48 but leave the LBA48 capacity empty.
         */
        if(total_sectors == 0) {
            total_sectors = ((uint32_t) identify_data[60])
                          | ((uint32_t) identify_data[61] << 16);
        }
    } else {
        /*
         * If LBA is not supported, the drive's size is calculated using CHS. The formula is:
         * Capacity = (Cyliners * Heads * Sectors) * 512 bytes
         */

        uint16_t cylinders = identify_data[1];
        uint16_t heads = identify_data[3];
        uint16_t sectors = identify_data[6];

        total_sectors = (uint64_t) cylinders * heads * sectors;
    }

    if(total_sectors > ATA_MAX_ADDRESSABLE_SECTORS) {
        kmessage(KMESSAGE_LEVEL_WARN, "ATA drive exceeds the addressable range, reported size clamped");

        total_sectors = ATA_MAX_ADDRESSABLE_SECTORS;
    }

    device->size = (uint32_t) (total_sectors * ATA_SECTOR_SIZE);

    device->present = true;

    return true;
}

size_t ata_write(ata_device_t* device, size_t offset, size_t size, char* buffer) {
    if(device->size < offset + size) {
        return 0;
    }

    if(!device->present) {
        return 0;
    }

    if(!device->lba_supported) {
        return 0;
    }

    size_t start_sector = offset / ATA_SECTOR_SIZE;
    size_t start_sector_offset = offset % ATA_SECTOR_SIZE;

    size_t end_sector = (offset + size - 1) / ATA_SECTOR_SIZE;
    size_t end_sector_offset = (offset + size - 1) % ATA_SECTOR_SIZE;

    uint8_t* sector_buffer = (uint8_t*) kmalloc(ATA_SECTOR_SIZE);

    if(!sector_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    char* buffer_pointer = buffer;
    size_t write_offset = 0;
    size_t write_size = 0;
    size_t total_size = 0;

    for(size_t sector_index = start_sector; sector_index <= end_sector; sector_index++) {
        // Read whole sector from the drive
        ata_read_sector_lba28(device, sector_index, sector_buffer);

        if(sector_index == start_sector) {
            write_offset = start_sector_offset;
            write_size = ATA_SECTOR_SIZE - write_offset;
        }

        if(sector_index == end_sector) {
            write_size = end_sector_offset - write_offset + 1;
        }

        // Alter the sector buffer
        memcpy(sector_buffer + write_offset, buffer_pointer, write_size);

        // Write whole sector back to the drive
        ata_write_sector_lba28(device, sector_index, sector_buffer);

        buffer_pointer = (char*) (((uintptr_t) buffer_pointer) + write_size);
        total_size += write_size;
    }

    kfree(sector_buffer);

    return total_size;
}

size_t ata_read(ata_device_t* device, size_t offset, size_t size, char* buffer) {
    if(device->size < offset + size) {
        return 0;
    }

    if(!device->present) {
        return 0;
    }

    if(!device->lba_supported) {
        return 0;
    }

    size_t start_sector = offset / ATA_SECTOR_SIZE;
    size_t start_sector_offset = offset % ATA_SECTOR_SIZE;

    size_t end_sector = (offset + size - 1) / ATA_SECTOR_SIZE;
    size_t end_sector_offset = (offset + size - 1) % ATA_SECTOR_SIZE;

    uint8_t* sector_buffer = (uint8_t*) kmalloc(ATA_SECTOR_SIZE);

    if(!sector_buffer) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    char* buffer_pointer = buffer;
    size_t read_offset = 0;
    size_t read_size = 0;
    size_t total_size = 0;

    for(size_t sector_index = start_sector; sector_index <= end_sector; sector_index++) {
        // Read whole sector from the drive
        ata_read_sector_lba28(device, sector_index, sector_buffer);

        if(sector_index == start_sector) {
            read_offset = start_sector_offset;
            read_size = ATA_SECTOR_SIZE - read_offset;
        }

        if(sector_index == end_sector) {
            read_size = end_sector_offset - read_offset + 1;
        }

        // Copy the sector buffer to the output buffer
        memcpy(buffer_pointer, sector_buffer + read_offset, read_size);

        buffer_pointer = (char*) (((uintptr_t) buffer_pointer) + read_size);
        total_size += read_size;
    }

    kfree(sector_buffer);

    return total_size;
}

/**
 * Block until the drive clears BSY. Its registers must not be touched while it is set, so every
 * command has to start and end with this.
 */
static void ata_wait_busy(uint16_t io_base) {
    while(inb(io_base + ATA_STATUS_REGISTER) & ATA_STATUS_BSY);
}

/**
 * Wait for the drive to announce that a block of data can be transferred.
 *
 * @return True once DRQ is set, false if the drive reported an error instead.
 */
static bool ata_wait_data(uint16_t io_base) {
    for(;;) {
        uint8_t status = inb(io_base + ATA_STATUS_REGISTER);

        if(status & (ATA_STATUS_ERR | ATA_STATUS_DF)) {
            return false;
        }

        if(!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRQ)) {
            return true;
        }
    }
}

/**
 * Select a drive and program the LBA registers for a single-sector transfer. Selecting a drive only
 * takes effect after a short settling time, for which reading the status register four times is the
 * conventional stand-in.
 */
static void ata_select_sector_lba28(uint16_t io_base, uint32_t lba) {
    ata_wait_busy(io_base);

    // Choose master/slave drives and set the LBA mode
    outb(io_base + ATA_DRIVE_REGISTER, 0xE0 | ((lba >> 24) & 0x0F));

    for(uint8_t i = 0; i < 4; i++) {
        inb(io_base + ATA_STATUS_REGISTER);
    }

    // Set number of sectors to transfer
    outb(io_base + ATA_SECTOR_COUNT_REGISTER, 0x01);

    // Set the LBA
    outb(io_base + ATA_LBA_LOW_REGISTER, (lba & 0x000000FF) >> 0);
    outb(io_base + ATA_LBA_MID_REGISTER, (lba & 0x0000FF00) >> 8);
    outb(io_base + ATA_LBA_HIGH_REGISTER, (lba & 0x00FF0000) >> 16);
}

static void ata_write_sector_lba28(ata_device_t* device, uint32_t lba, uint8_t* buffer) {
    uint16_t io_base = ata_get_io_base(device);

    ata_select_sector_lba28(io_base, lba);

    outb(io_base + ATA_COMMAND_REGISTER, ATA_COMMAND_WRITE_SECTORS);

    // The drive raises DRQ once it is ready to take the data. Pushing it out earlier loses it.
    if(!ata_wait_data(io_base)) {
        return;
    }

    for(uint16_t i = 0; i < 256; i++) {
        outw(io_base + ATA_DATA_REGISTER, ((uint16_t*) buffer)[i]);
    }

    /*
     * The transfer only queues the sector. Flushing the cache waits for it to reach the medium,
     * which also keeps the next command from programming the registers while this write is still
     * in flight -- writing a 1 KiB block means two of these back to back.
     */
    ata_wait_busy(io_base);

    outb(io_base + ATA_COMMAND_REGISTER, ATA_COMMAND_CACHE_FLUSH);

    ata_wait_busy(io_base);
}

static void ata_read_sector_lba28(ata_device_t* device, uint32_t lba, uint8_t* buffer) {
    uint16_t io_base = ata_get_io_base(device);

    ata_select_sector_lba28(io_base, lba);

    outb(io_base + ATA_COMMAND_REGISTER, ATA_COMMAND_READ_SECTORS);

    if(!ata_wait_data(io_base)) {
        return;
    }

    for(uint16_t i = 0; i < 256; i++) {
        ((uint16_t*) buffer)[i] = inw(io_base + ATA_DATA_REGISTER);
    }
}
