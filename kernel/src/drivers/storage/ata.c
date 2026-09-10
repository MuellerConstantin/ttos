#include <drivers/storage/ata.h>
#include <memory/kheap.h>
#include <system/kpanic.h>
#include <system/kmessage.h>
#include <device/device.h>
#include <drivers/pci/pci.h>
#include <drivers/pci/types.h>

/** Names of the four drives of a controller, in the order of ata_drive_t. */
static const char* ata_drive_names[4] = {
    "ATA Primary Master Drive",
    "ATA Primary Slave Drive",
    "ATA Secondary Master Drive",
    "ATA Secondary Slave Drive"
};

static void ata_claim_controller(device_t* controller);
static void ata_channel_ports(device_t* controller, ata_channel_t* primary, ata_channel_t* secondary);
static void ata_channel_disable_interrupts(const ata_channel_t* channel);
static uint16_t ata_native_port(pci_device_t* pci_device, uint8_t bar_index);
static void ata_register_drive(device_t* controller, ata_device_t* drive);
static bool ata_is_master(ata_device_t* device);
static bool ata_device_probe(ata_device_t* device);
static bool ata_wait_busy(uint16_t io_base, uint32_t timeout);
static bool ata_wait_data(uint16_t io_base, uint32_t timeout);
static bool ata_select_sector_lba28(uint16_t io_base, uint32_t lba);
static bool ata_write_sector_lba28(ata_device_t* device, uint32_t lba, uint8_t* buffer);
static bool ata_read_sector_lba28(ata_device_t* device, uint32_t lba, uint8_t* buffer);

static size_t ata_driver_sector_size(device_t* device);
static size_t ata_driver_total_size(device_t* device);
static size_t ata_driver_read(device_t* device, size_t offset, size_t size, char* buffer);
static size_t ata_driver_write(device_t* device, size_t offset, size_t size, char* buffer);

int32_t ata_init() {
    linked_list_t* controllers = pci_find_all_devices(PCI_TYPE_MASS_STORAGE_CONTROLLER, PCI_SUBTYPE_IDE_CONTROLLER);

    if(!controllers) {
        return -1;
    }

    /*
     * A chipset that carries a PATA and a SATA controller side by side reports
     * two IDE controllers, and drives can sit on either of them. Every one of
     * them is probed, and every drive found gets registered below the
     * controller it hangs off.
     */
    linked_list_foreach(controllers, node) {
        device_t* controller = (device_t*) node->data;

        ata_channel_t primary;
        ata_channel_t secondary;

        ata_channel_ports(controller, &primary, &secondary);

        /*
         * Both channels are silenced before the first command goes out. A drive
         * that raises an interrupt nobody handles takes the machine with it.
         */
        ata_channel_disable_interrupts(&primary);
        ata_channel_disable_interrupts(&secondary);

        bool claimed = false;

        for(size_t index = 0; index < 4; index++) {
            const ata_channel_t* channel = (index <= ATA_PRIMARY_SLAVE_DRIVE) ? &primary : &secondary;

            ata_device_t* drive = (ata_device_t*) kmalloc(sizeof(ata_device_t));

            if(!drive) {
                KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
            }

            drive->drive = (ata_drive_t) index;
            drive->io_base = channel->io_base;
            drive->control_base = channel->control_base;
            drive->present = false;
            drive->lba_supported = false;
            drive->lba48_supported = false;
            drive->size = 0;

            if(!ata_device_probe(drive)) {
                kfree(drive);

                continue;
            }

            // Nothing marks the controller as driven until one of its drives answers.
            if(!claimed) {
                ata_claim_controller(controller);

                claimed = true;
            }

            ata_register_drive(controller, drive);
        }
    }

    linked_list_destroy(controllers, false);

    kmessage(KMESSAGE_LEVEL_INFO, "ata: Done probing IDE controllers");

    return 0;
}

/**
 * Registers a drive that answered the probe as a storage device below the
 * controller it hangs off.
 */
static void ata_register_drive(device_t* controller, ata_device_t* drive) {
    storage_device_t* device = (storage_device_t*) kmalloc(sizeof(storage_device_t));

    if(!device) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->name = (char*) kmalloc(ATA_DRIVE_NAME_LENGTH);

    if(!device->name) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    /*
     * The address of the controller belongs in the name: a board with two IDE
     * controllers has two primary master drives, and a volume listing has
     * nothing but the name to tell the volumes of the two apart.
     */
    pci_device_t* pci_device = (pci_device_t*) controller->bus.data;

    strfmt(device->name, "%s (%d:%d.%d)", ata_drive_names[drive->drive], pci_device->bus, pci_device->slot, pci_device->function);

    device_generate_id(device->id);
    device->type = DEVICE_TYPE_STORAGE;
    device->bus.type = DEVICE_BUS_TYPE_ATA;
    device->bus.data = drive;

    device->driver.storage = (storage_driver_t*) kmalloc(sizeof(storage_driver_t));

    if(!device->driver.storage) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    device->driver.storage->sector_size = ata_driver_sector_size;
    device->driver.storage->total_size = ata_driver_total_size;
    device->driver.storage->read = ata_driver_read;
    device->driver.storage->write = ata_driver_write;

    /*
     * Registering hands the drive to the volume manager, which reads its
     * partition table right away. Saying so beforehand keeps a drive that stops
     * answering halfway from looking like a stall of unknown origin.
     */
    char* kernel_message = (char*) kmalloc(ATA_MESSAGE_LENGTH);

    if(!kernel_message) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strfmt(kernel_message, "ata: Registering %s on port %x", device->name, drive->io_base);

    kmessage(KMESSAGE_LEVEL_INFO, kernel_message);

    device_register(controller, device);
}

static size_t ata_driver_sector_size(device_t* device) {
    (void) device;

    return ATA_SECTOR_SIZE;
}

static size_t ata_driver_total_size(device_t* device) {
    return ((ata_device_t*) device->bus.data)->size;
}

static size_t ata_driver_read(device_t* device, size_t offset, size_t size, char* buffer) {
    return ata_read((ata_device_t*) device->bus.data, offset, size, buffer);
}

static size_t ata_driver_write(device_t* device, size_t offset, size_t size, char* buffer) {
    return ata_write((ata_device_t*) device->bus.data, offset, size, buffer);
}

/**
 * Marks a controller as one this driver drives. It keeps the name the PCI scan
 * gave it, which carries its address and therefore already tells two IDE
 * controllers of the same board apart.
 */
static void ata_claim_controller(device_t* controller) {
    controller->type = DEVICE_TYPE_CONTROLLER;
}

/**
 * Works out which ports the two channels of a controller answer on.
 */
static void ata_channel_ports(device_t* controller, ata_channel_t* primary, ata_channel_t* secondary) {
    pci_device_t* pci_device = (pci_device_t*) controller->bus.data;

    /*
     * Whether a channel listens on the legacy ports or on the ports its BARs
     * were assigned is the controller's own decision, reported in prog_if.
     * Assuming the legacy ports finds nothing on a controller in native mode.
     */
    if(pci_device->prog_if & ATA_PROG_IF_PRIMARY_NATIVE) {
        primary->io_base = ata_native_port(pci_device, ATA_PRIMARY_COMMAND_BAR);
        primary->control_base = ata_native_port(pci_device, ATA_PRIMARY_CONTROL_BAR);

        if(primary->control_base) {
            primary->control_base += ATA_CONTROL_BAR_OFFSET;
        }
    } else {
        primary->io_base = ATA_PRIMARY_IO_BASE;
        primary->control_base = ATA_PRIMARY_CONTROL_BASE;
    }

    if(pci_device->prog_if & ATA_PROG_IF_SECONDARY_NATIVE) {
        secondary->io_base = ata_native_port(pci_device, ATA_SECONDARY_COMMAND_BAR);
        secondary->control_base = ata_native_port(pci_device, ATA_SECONDARY_CONTROL_BAR);

        if(secondary->control_base) {
            secondary->control_base += ATA_CONTROL_BAR_OFFSET;
        }
    } else {
        secondary->io_base = ATA_SECONDARY_IO_BASE;
        secondary->control_base = ATA_SECONDARY_CONTROL_BASE;
    }

    char* kernel_message = (char*) kmalloc(ATA_MESSAGE_LENGTH);

    if(!kernel_message) {
        KPANIC(KPANIC_KHEAP_OUT_OF_MEMORY_CODE, KPANIC_KHEAP_OUT_OF_MEMORY_MESSAGE, NULL);
    }

    strfmt(kernel_message, "ata: Probing IDE controller (%d:%d.%d) with prog_if %x, primary %x/%x, secondary %x/%x",
           pci_device->bus, pci_device->slot, pci_device->function, pci_device->prog_if,
           primary->io_base, primary->control_base, secondary->io_base, secondary->control_base);

    kmessage(KMESSAGE_LEVEL_INFO, kernel_message);
}

/**
 * Tells the drives of a channel to keep their interrupts to themselves. The
 * driver polls, and an interrupt raised for it would be acknowledged by nobody.
 */
static void ata_channel_disable_interrupts(const ata_channel_t* channel) {
    if(channel->control_base == 0) {
        return;
    }

    outb(channel->control_base + ATA_DEVICE_CONTROL_REGISTER, ATA_DEVICE_CONTROL_NIEN);
}

static uint16_t ata_native_port(pci_device_t* pci_device, uint8_t bar_index) {
    if(pci_load_bar_info(pci_device, bar_index) != 0) {
        return 0;
    }

    // The BAR is read out by value, a packed struct has no address worth taking.
    uint8_t type = pci_device->data.general.bar[bar_index].type;
    uint16_t io_port = pci_device->data.general.bar[bar_index].io_port;

    // A BAR the firmware never assigned leaves the channel without ports.
    if(type != PCI_BAR_IO_SPACE || io_port == 0) {
        return 0;
    }

    return io_port;
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
    uint16_t io_base = device->io_base;
    bool is_master = ata_is_master(device);

    // A channel whose ports the controller does not decode has nothing to find.
    if(io_base == 0) {
        return false;
    }

    // Choose master/slave drive
    outb(io_base + ATA_DRIVE_REGISTER, is_master ? 0xA0 : 0xB0);

    // Set the sector count and LBA registers to 0
    outb(io_base + ATA_SECTOR_COUNT_REGISTER, 0x00);
    outb(io_base + ATA_LBA_LOW_REGISTER, 0x00);
    outb(io_base + ATA_LBA_MID_REGISTER, 0x00);
    outb(io_base + ATA_LBA_HIGH_REGISTER, 0x00);

    // Send the identify command
    outb(io_base + ATA_COMMAND_REGISTER, 0xEC);

    uint8_t status = inb(io_base + ATA_STATUS_REGISTER);

    /*
     * A status of zero is an empty drive slot. All ones means nobody drives
     * these ports at all: an unconnected bus floats high, and taking that for a
     * busy drive would leave the boot spinning in the wait below.
     */
    if(status == 0x00 || status == 0xFF) {
        return false;
    }

    // Wait for the drive to be ready
    if(!ata_wait_busy(io_base, ATA_PROBE_TIMEOUT)) {
        return false;
    }

    // Check if the drive is an ATA drive
    if(inb(io_base + ATA_LBA_MID_REGISTER) != 0x00 || inb(io_base + ATA_LBA_HIGH_REGISTER) != 0x00) {
        return false;
    }

    uint8_t drq = 0;
    uint8_t err = 0;

    // Wait for the drive to hand over its identification data
    for(uint32_t attempt = 0; !drq && !err && attempt < ATA_PROBE_TIMEOUT; attempt++) {
        drq = inb(io_base + ATA_STATUS_REGISTER) & ATA_STATUS_DRQ;
        err = inb(io_base + ATA_STATUS_REGISTER) & ATA_STATUS_ERR;
    }

    if(err || !drq) {
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
        // A drive that stops answering ends the transfer, the caller gets what arrived.
        if(!ata_read_sector_lba28(device, sector_index, sector_buffer)) {
            break;
        }

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
        if(!ata_write_sector_lba28(device, sector_index, sector_buffer)) {
            break;
        }

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
        // A drive that stops answering ends the transfer, the caller gets what arrived.
        if(!ata_read_sector_lba28(device, sector_index, sector_buffer)) {
            break;
        }

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
 * Wait for the drive to clear BSY. Its registers must not be touched while it is set, so every
 * command has to start and end with this.
 *
 * @return True once the drive went idle, false when it never did.
 */
static bool ata_wait_busy(uint16_t io_base, uint32_t timeout) {
    for(uint32_t attempt = 0; attempt < timeout; attempt++) {
        if((inb(io_base + ATA_STATUS_REGISTER) & ATA_STATUS_BSY) == 0) {
            return true;
        }
    }

    return false;
}

/**
 * Wait for the drive to announce that a block of data can be transferred.
 *
 * @return True once DRQ is set, false if the drive reported an error or never answered.
 */
static bool ata_wait_data(uint16_t io_base, uint32_t timeout) {
    for(uint32_t attempt = 0; attempt < timeout; attempt++) {
        uint8_t status = inb(io_base + ATA_STATUS_REGISTER);

        if(status & (ATA_STATUS_ERR | ATA_STATUS_DF)) {
            return false;
        }

        if(!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRQ)) {
            return true;
        }
    }

    return false;
}

/**
 * Select a drive and program the LBA registers for a single-sector transfer. Selecting a drive only
 * takes effect after a short settling time, for which reading the status register four times is the
 * conventional stand-in.
 */
static bool ata_select_sector_lba28(uint16_t io_base, uint32_t lba) {
    if(!ata_wait_busy(io_base, ATA_COMMAND_TIMEOUT)) {
        return false;
    }

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

    return true;
}

static bool ata_write_sector_lba28(ata_device_t* device, uint32_t lba, uint8_t* buffer) {
    uint16_t io_base = device->io_base;

    if(!ata_select_sector_lba28(io_base, lba)) {
        return false;
    }

    outb(io_base + ATA_COMMAND_REGISTER, ATA_COMMAND_WRITE_SECTORS);

    // The drive raises DRQ once it is ready to take the data. Pushing it out earlier loses it.
    if(!ata_wait_data(io_base, ATA_COMMAND_TIMEOUT)) {
        return false;
    }

    for(uint16_t i = 0; i < 256; i++) {
        outw(io_base + ATA_DATA_REGISTER, ((uint16_t*) buffer)[i]);
    }

    /*
     * The transfer only queues the sector. Flushing the cache waits for it to reach the medium,
     * which also keeps the next command from programming the registers while this write is still
     * in flight -- writing a 1 KiB block means two of these back to back.
     */
    if(!ata_wait_busy(io_base, ATA_COMMAND_TIMEOUT)) {
        return false;
    }

    outb(io_base + ATA_COMMAND_REGISTER, ATA_COMMAND_CACHE_FLUSH);

    return ata_wait_busy(io_base, ATA_COMMAND_TIMEOUT);
}

static bool ata_read_sector_lba28(ata_device_t* device, uint32_t lba, uint8_t* buffer) {
    uint16_t io_base = device->io_base;

    if(!ata_select_sector_lba28(io_base, lba)) {
        return false;
    }

    outb(io_base + ATA_COMMAND_REGISTER, ATA_COMMAND_READ_SECTORS);

    if(!ata_wait_data(io_base, ATA_COMMAND_TIMEOUT)) {
        return false;
    }

    for(uint16_t i = 0; i < 256; i++) {
        ((uint16_t*) buffer)[i] = inw(io_base + ATA_DATA_REGISTER);
    }

    return true;
}
