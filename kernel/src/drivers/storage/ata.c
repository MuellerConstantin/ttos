#include <drivers/storage/ata.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <device/device.h>

static ata_device_t ata_devices[4] = {
    {ATA_PRIMARY_MASTER_DRIVE, 0, false, false, false},
    {ATA_PRIMARY_SLAVE_DRIVE, 0, false, false, false},
    {ATA_SECONDARY_MASTER_DRIVE, 0, false, false, false},
    {ATA_SECONDARY_SLAVE_DRIVE, 0, false, false, false}
};

static uint16_t ata_get_io_base(ata_device_t* device);
static bool ata_is_master(ata_device_t* device);
static bool ata_device_probe(ata_device_t* device);
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

int32_t ata_init() {
    if(ata_device_probe(&ata_devices[ATA_PRIMARY_MASTER_DRIVE])) {
        storage_device_t *device = (storage_device_t*) kmalloc(sizeof(storage_device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->info.name = (char*) kmalloc(25);

        if(!device->info.name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        generate_uuid_v4(&device->info.id);
        strcpy(device->info.name, "ATA Primary Master Drive");
        device->info.type = DEVICE_TYPE_STORAGE;
        device->info.bus.type = DEVICE_BUS_TYPE_PLATFORM;
        device->info.bus.data = NULL;

        device->driver = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

        if(!device->driver) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->driver->total_size = ata_total_size_primary_master;
        device->driver->read = ata_read_primary_master;
        device->driver->write = ata_write_primary_master;

        device_register(NULL, device);
    }

    if(ata_device_probe(&ata_devices[ATA_PRIMARY_SLAVE_DRIVE])) {
        storage_device_t *device = (storage_device_t*) kmalloc(sizeof(storage_device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->info.name = (char*) kmalloc(24);

        if(!device->info.name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        generate_uuid_v4(&device->info.id);
        strcpy(device->info.name, "ATA Primary Slave Drive");
        device->info.type = DEVICE_TYPE_STORAGE;
        device->info.bus.type = DEVICE_BUS_TYPE_PLATFORM;
        device->info.bus.data = NULL;

        device->driver = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

        if(!device->driver) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->driver->total_size = ata_total_size_primary_slave;
        device->driver->read = ata_read_primary_slave;
        device->driver->write = ata_write_primary_slave;

        device_register(NULL, device);
    }

    if(ata_device_probe(&ata_devices[ATA_SECONDARY_MASTER_DRIVE])) {
        storage_device_t *device = (storage_device_t*) kmalloc(sizeof(storage_device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->info.name = (char*) kmalloc(27);

        if(!device->info.name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        generate_uuid_v4(&device->info.id);
        strcpy(device->info.name, "ATA Secondary Master Drive");
        device->info.type = DEVICE_TYPE_STORAGE;
        device->info.bus.type = DEVICE_BUS_TYPE_PLATFORM;
        device->info.bus.data = NULL;

        device->driver = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

        if(!device->driver) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->driver->total_size = ata_total_size_secondary_master;
        device->driver->read = ata_read_secondary_master;
        device->driver->write = ata_write_secondary_master;

        device_register(NULL, device);
    }

    if(ata_device_probe(&ata_devices[ATA_SECONDARY_SLAVE_DRIVE])) {
        storage_device_t *device = (storage_device_t*) kmalloc(sizeof(storage_device_t));

        if(!device) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->info.name = (char*) kmalloc(26);

        if(!device->info.name) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        generate_uuid_v4(&device->info.id);
        strcpy(device->info.name, "ATA Secondary Slave Drive");
        device->info.type = DEVICE_TYPE_STORAGE;
        device->info.bus.type = DEVICE_BUS_TYPE_PLATFORM;
        device->info.bus.data = NULL;

        device->driver = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

        if(!device->driver) {
            KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
        }

        device->driver->total_size = ata_total_size_secondary_slave;
        device->driver->read = ata_read_secondary_slave;
        device->driver->write = ata_write_secondary_slave;

        device_register(NULL, device);
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

    // Check if LBA48 is supported
    if(identify_data[83] & (1 << 10)) {
        device->lba48_supported = true;
    }

    // Get the drive's size
    if(device->lba_supported) {
        if(device->lba48_supported) {
            device->size = (identify_data[100] | (identify_data[101] << 16) | (identify_data[102] << 32) | (identify_data[103] << 48)) * ATA_SECTOR_SIZE;
        } else {
            device->size = (identify_data[60] | (identify_data[61] << 16)) * ATA_SECTOR_SIZE;
        }
    } else {
        /*
         * If LBA is not supported, the drive's size is calculated using CHS. The formula is:
         * Capacity = (Cyliners * Heads * Sectors) * 512 bytes
         */

        uint16_t cylinders = identify_data[1];
        uint16_t heads = identify_data[3];
        uint16_t sectors = identify_data[6];
        
        device->size = (cylinders * heads * sectors) * ATA_SECTOR_SIZE;
    }

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

static void ata_write_sector_lba28(ata_device_t* device, uint32_t lba, uint8_t* buffer) {
    uint16_t io_base = ata_get_io_base(device);

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

static void ata_read_sector_lba28(ata_device_t* device, uint32_t lba, uint8_t* buffer) {
    uint16_t io_base = ata_get_io_base(device);

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
