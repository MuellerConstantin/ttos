#include <drivers/storage/ata.h>
#include <memory/kheap.h>
#include <sys/kpanic.h>
#include <device/device.h>
#include <drivers/serial/uart/16550.h>

static uint16_t ata_get_io_base(ata_drive_t drive);
static bool ata_is_master(ata_drive_t drive);
static bool ata_drive_probe(ata_drive_t drive);
static void ata_write_sector(ata_drive_t drive, uint32_t lba, uint8_t* buffer);
static void ata_read_sector(ata_drive_t drive, uint32_t lba, uint8_t* buffer);

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

int32_t ata_init() {
    if(ata_drive_probe(ATA_PRIMARY_MASTER_DRIVE)) {
        device_t *device = (device_t*) kmalloc(sizeof(device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->name = (char*) kmalloc(25);

        if(!device->name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        strcpy(device->name, "ATA Primary Master Drive");
        device->type = DEVICE_TYPE_STORAGE;
        device->bus.type = DEVICE_BUS_TYPE_PLATFORM;
        device->bus.data = NULL;

        device->driver.storage = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

        if(!device->driver.storage) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->driver.storage->total_size = ata_total_size_primary_master;
        device->driver.storage->read = ata_read_primary_master;
        device->driver.storage->write = ata_write_primary_master;

        device_register(NULL, device);
    }

    if(ata_drive_probe(ATA_PRIMARY_SLAVE_DRIVE)) {
        device_t *device = (device_t*) kmalloc(sizeof(device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->name = (char*) kmalloc(24);

        if(!device->name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        strcpy(device->name, "ATA Primary Slave Drive");
        device->type = DEVICE_TYPE_STORAGE;
        device->bus.type = DEVICE_BUS_TYPE_PLATFORM;
        device->bus.data = NULL;

        device->driver.storage = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

        if(!device->driver.storage) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->driver.storage->total_size = ata_total_size_primary_slave;
        device->driver.storage->read = ata_read_primary_slave;
        device->driver.storage->write = ata_write_primary_slave;

        device_register(NULL, device);
    }

    if(ata_drive_probe(ATA_SECONDARY_MASTER_DRIVE)) {
        device_t *device = (device_t*) kmalloc(sizeof(device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->name = (char*) kmalloc(27);

        if(!device->name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        strcpy(device->name, "ATA Secondary Master Drive");
        device->type = DEVICE_TYPE_STORAGE;
        device->bus.type = DEVICE_BUS_TYPE_PLATFORM;
        device->bus.data = NULL;

        device->driver.storage = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

        if(!device->driver.storage) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->driver.storage->total_size = ata_total_size_secondary_master;
        device->driver.storage->read = ata_read_secondary_master;
        device->driver.storage->write = ata_write_secondary_master;

        device_register(NULL, device);
    }

    if(ata_drive_probe(ATA_SECONDARY_SLAVE_DRIVE)) {
        device_t *device = (device_t*) kmalloc(sizeof(device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->name = (char*) kmalloc(26);

        if(!device->name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        strcpy(device->name, "ATA Secondary Slave Drive");
        device->type = DEVICE_TYPE_STORAGE;
        device->bus.type = DEVICE_BUS_TYPE_PLATFORM;
        device->bus.data = NULL;

        device->driver.storage = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

        if(!device->driver.storage) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->driver.storage->total_size = ata_total_size_secondary_slave;
        device->driver.storage->read = ata_read_secondary_slave;
        device->driver.storage->write = ata_write_secondary_slave;

        device_register(NULL, device);
    }

    return 0;
}

static size_t ata_total_size_primary_master() {
    return 0;
}

static size_t ata_write_primary_master(size_t offset, size_t size, char* buffer) {
    return ata_write(ATA_PRIMARY_MASTER_DRIVE, offset, size, buffer);
}

static size_t ata_read_primary_master(size_t offset, size_t size, char* buffer) {
    return ata_read(ATA_PRIMARY_MASTER_DRIVE, offset, size, buffer);
}

static size_t ata_total_size_primary_slave() {
    return 0;
}

static size_t ata_write_primary_slave(size_t offset, size_t size, char* buffer) {
    return ata_write(ATA_PRIMARY_SLAVE_DRIVE, offset, size, buffer);
}

static size_t ata_read_primary_slave(size_t offset, size_t size, char* buffer) {
    return ata_read(ATA_PRIMARY_SLAVE_DRIVE, offset, size, buffer);
}

static size_t ata_total_size_secondary_master() {
    return 0;
}

static size_t ata_write_secondary_master(size_t offset, size_t size, char* buffer) {
    return ata_write(ATA_SECONDARY_MASTER_DRIVE, offset, size, buffer);
}

static size_t ata_read_secondary_master(size_t offset, size_t size, char* buffer) {
    return ata_read(ATA_SECONDARY_MASTER_DRIVE, offset, size, buffer);
}

static size_t ata_total_size_secondary_slave() {
    return 0;
}

static size_t ata_write_secondary_slave(size_t offset, size_t size, char* buffer) {
    return ata_write(ATA_SECONDARY_SLAVE_DRIVE, offset, size, buffer);
}

static size_t ata_read_secondary_slave(size_t offset, size_t size, char* buffer) {
    return ata_read(ATA_SECONDARY_SLAVE_DRIVE, offset, size, buffer);
}

static uint16_t ata_get_io_base(ata_drive_t drive) {
    switch(drive) {
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

static bool ata_is_master(ata_drive_t drive) {
    switch(drive) {
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

static bool ata_drive_probe(ata_drive_t drive) {
    uint16_t io_base = ata_get_io_base(drive);
    bool is_master = ata_is_master(drive);

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

    // Read the drive's identification data
    for(uint16_t i = 0; i < 256; i++) {
        inw(io_base + ATA_DATA_REGISTER);
    }

    return true;
}

size_t ata_write(ata_drive_t drive, size_t offset, size_t size, char* buffer) {
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
        ata_read_sector(drive, sector_index, sector_buffer);

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
        ata_write_sector(drive, sector_index, sector_buffer);

        buffer_pointer = (char*) (((uintptr_t) buffer_pointer) + write_size);
        total_size += write_size;
    }

    kfree(sector_buffer);

    return total_size;
}

size_t ata_read(ata_drive_t drive, size_t offset, size_t size, char* buffer) {
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
        ata_read_sector(drive, sector_index, sector_buffer);

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

static void ata_write_sector(ata_drive_t drive, uint32_t lba, uint8_t* buffer) {
    uint16_t io_base = ata_get_io_base(drive);

    // Choose master/slave drives and set the LBA mode
    outb(io_base + ATA_DRIVE_REGISTER, 0xE0 | ((lba >> 24) & 0x0F));

    // Set number of sectors to write
    outb(io_base + ATA_SECTOR_COUNT_REGISTER, 0x01);

    // Set the LBA
    outb(io_base + ATA_LBA_LOW_REGISTER, (lba & 0x000000FF) >> 0);
    outb(io_base + ATA_LBA_MID_REGISTER, (lba & 0x0000FF00) >> 8);
    outb(io_base + ATA_LBA_HIGH_REGISTER, (lba & 0x00FF0000) >> 16);

    // Send the write command
    outb(io_base + ATA_COMMAND_REGISTER, 0x30);

    // Wait for the drive to be ready
    while(inb(io_base + ATA_STATUS_REGISTER) & 0x80);

    // Write the sector
    for(uint16_t i = 0; i < 256; i++) {
        outw(io_base + ATA_DATA_REGISTER, ((uint16_t*) buffer)[i]);
    }
}

static void ata_read_sector(ata_drive_t drive, uint32_t lba, uint8_t* buffer) {
    uint16_t io_base = ata_get_io_base(drive);

    // Choose master/slave drives and set the LBA mode
    outb(io_base + ATA_DRIVE_REGISTER, 0xE0 | ((lba >> 24) & 0x0F));

    // Set number of sectors to read
    outb(io_base + ATA_SECTOR_COUNT_REGISTER, 0x01);

    // Set the LBA
    outb(io_base + ATA_LBA_LOW_REGISTER, (lba & 0x000000FF) >> 0);
    outb(io_base + ATA_LBA_MID_REGISTER, (lba & 0x0000FF00) >> 8);
    outb(io_base + ATA_LBA_HIGH_REGISTER, (lba & 0x00FF0000) >> 16);

    // Send the read command
    outb(io_base + ATA_COMMAND_REGISTER, 0x20);

    // Wait for the drive to be ready
    while(inb(io_base + ATA_STATUS_REGISTER) & 0x80);

    // Read the sector
    for(uint16_t i = 0; i < 256; i++) {
        ((uint16_t*) buffer)[i] = inw(io_base + ATA_DATA_REGISTER);
    }
}
