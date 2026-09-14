#include <devio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** A master boot record holds four partition entries, which is what limits a disk to four. */
#define MKPART_MAX_PARTITIONS 4

/** The master boot record is the first sector of the disk and always 512 bytes wide. */
#define MKPART_MBR_SIZE 512

/** Offset of the partition table inside it, and of the signature behind it. */
#define MKPART_TABLE_OFFSET 0x1BE
#define MKPART_SIGNATURE_OFFSET 0x1FE
#define MKPART_SIGNATURE 0xAA55

/** Partition type the file system of this system is marked with. */
#define MKPART_TYPE_LINUX 0x83

/*
 * Status byte of a partition that is marked active, and of one that is not. The
 * flag says which partition a master boot record hands control to; a disk that
 * is not booted from has no use for it, so none is set unless it is asked for.
 */
#define MKPART_STATUS_ACTIVE 0x80
#define MKPART_STATUS_INACTIVE 0x00

/*
 * Geometry the CHS fields of an entry are computed with. The fields are a
 * leftover from before LBA addressing and are ignored by anything that can
 * address a disk linearly, but tools still fill them, so a table written here
 * looks like one written anywhere else.
 */
#define MKPART_HEADS 255
#define MKPART_SECTORS_PER_TRACK 63

/** Beyond this cylinder a CHS address does not fit into its three bytes. */
#define MKPART_MAX_CYLINDER 1023

/** Partitions start on a full mebibyte, in 512 byte sectors. */
#define MKPART_ALIGNMENT 2048

/*
 * Bytes zeroed at the start of every new partition. A file system left there by
 * an earlier layout would otherwise still be found: probing reads a superblock
 * from a fixed offset and does not care that the partition around it is new.
 */
#define MKPART_WIPE_SIZE 4096

typedef struct {
    uint32_t start;
    uint32_t sectors;
} mkpart_partition_t;

static unsigned char mbr[MKPART_MBR_SIZE];
static unsigned char wipe[MKPART_WIPE_SIZE];

/*
 * Reads a size in bytes: a decimal number with an optional K, M or G suffix, or
 * "rest" for whatever the disk has left. Returns 0 when the text is neither.
 */
static int mkpart_size(const char* text, uint32_t* out) {
    if (strcmp(text, "rest") == 0) {
        *out = 0;
        return 1;
    }

    char* end;
    uint32_t value = strtoul(text, &end, 10);

    if (end == text) {
        return 0;
    }

    switch (*end) {
        case 'G': case 'g': value *= 1024; // fall through
        case 'M': case 'm': value *= 1024; // fall through
        case 'K': case 'k': value *= 1024; end++; break;
        case '\0': break;
        default: return 0;
    }

    // Only the suffix may follow the digits.
    if (*end != '\0') {
        return 0;
    }

    *out = value;

    return 1;
}

/** Rounds a sector number up to the next alignment boundary. */
static uint32_t mkpart_align(uint32_t sector) {
    uint32_t remainder = sector % MKPART_ALIGNMENT;

    return remainder == 0 ? sector : sector + (MKPART_ALIGNMENT - remainder);
}

/*
 * Writes the three bytes of a CHS address: head, then the sector in the low six
 * bits with the upper two cylinder bits above it, then the low cylinder bits.
 * An address past the last representable cylinder is written as the largest one
 * there is, which is how a table says "read the LBA fields instead".
 */
static void mkpart_chs(unsigned char* out, uint32_t lba) {
    uint32_t cylinder = lba / (MKPART_HEADS * MKPART_SECTORS_PER_TRACK);
    uint32_t remainder = lba % (MKPART_HEADS * MKPART_SECTORS_PER_TRACK);
    uint32_t head = remainder / MKPART_SECTORS_PER_TRACK;
    uint32_t sector = remainder % MKPART_SECTORS_PER_TRACK + 1;

    if (cylinder > MKPART_MAX_CYLINDER) {
        out[0] = 0xFE;
        out[1] = 0xFF;
        out[2] = 0xFF;

        return;
    }

    out[0] = (unsigned char) head;
    out[1] = (unsigned char) ((sector & 0x3F) | ((cylinder >> 2) & 0xC0));
    out[2] = (unsigned char) (cylinder & 0xFF);
}

/** Writes a 32 bit value in the little endian order the table stores it in. */
static void mkpart_put32(unsigned char* out, uint32_t value) {
    out[0] = (unsigned char) (value & 0xFF);
    out[1] = (unsigned char) ((value >> 8) & 0xFF);
    out[2] = (unsigned char) ((value >> 16) & 0xFF);
    out[3] = (unsigned char) ((value >> 24) & 0xFF);
}

/** Whether the disk already carries a table with at least one partition in it. */
static int mkpart_has_partitions(const unsigned char* sector) {
    uint16_t signature = (uint16_t) (sector[MKPART_SIGNATURE_OFFSET] | (sector[MKPART_SIGNATURE_OFFSET + 1] << 8));

    if (signature != MKPART_SIGNATURE) {
        return 0;
    }

    for (int index = 0; index < MKPART_MAX_PARTITIONS; index++) {
        if (sector[MKPART_TABLE_OFFSET + index * 16 + 4] != 0) {
            return 1;
        }
    }

    return 0;
}

static void mkpart_usage(void) {
    puts("usage: mkpart [-f] [-b <n>] <device> <size> [<size> ...]\n");
    puts("       sizes are bytes with an optional K, M or G, or rest for what is left\n");
    puts("       -f overwrites an existing partition table\n");
    puts("       -b marks partition n active, which a disk that is booted from needs\n");
}

int main(int argc, char** argv) {
    int argument = 1;
    int force = 0;
    int active = 0;

    // Options come before the device, in any order.
    while (argument < argc && argv[argument][0] == '-') {
        if (strcmp(argv[argument], "-f") == 0) {
            force = 1;
            argument++;
            continue;
        }

        if (strcmp(argv[argument], "-b") == 0 && argument + 1 < argc) {
            active = argv[argument + 1][0] - '0';

            if (active < 1 || active > MKPART_MAX_PARTITIONS || argv[argument + 1][1] != '\0') {
                puts("mkpart: -b takes a partition number from 1 to 4\n");
                return 1;
            }

            argument += 2;
            continue;
        }

        mkpart_usage();
        return 1;
    }

    int count = argc - argument - 1;

    if (count < 1 || count > MKPART_MAX_PARTITIONS) {
        mkpart_usage();
        return 1;
    }

    if (active > count) {
        printf("mkpart: there is no partition %d to mark active\n", active);
        return 1;
    }

    const char* id = argv[argument++];
    storageinfo_t storage;

    if (devio_get_storageinfo(id, &storage) != 0) {
        printf("mkpart: no such storage device: %s\n", id);
        return 1;
    }

    if (storage.sector_size == 0) {
        printf("mkpart: %s has no sectors to partition\n", id);
        return 1;
    }

    uint32_t total_sectors = storage.size / storage.sector_size;
    mkpart_partition_t partitions[MKPART_MAX_PARTITIONS];
    uint32_t cursor = mkpart_align(TTOS_PARTITION_START);

    for (int index = 0; index < count; index++) {
        uint32_t size;

        if (!mkpart_size(argv[argument + index], &size)) {
            printf("mkpart: not a size: %s\n", argv[argument + index]);
            return 1;
        }

        if (cursor >= total_sectors) {
            puts("mkpart: the disk is full before the last partition\n");
            return 1;
        }

        uint32_t sectors = size == 0 ? total_sectors - cursor : size / storage.sector_size;

        // Only the last partition may take what is left, and it ends at the disk.
        if (size == 0 && index != count - 1) {
            puts("mkpart: only the last partition can be rest\n");
            return 1;
        }

        if (sectors == 0 || cursor + sectors > total_sectors) {
            printf("mkpart: partition %d does not fit\n", index + 1);
            return 1;
        }

        partitions[index].start = cursor;
        partitions[index].sectors = sectors;

        cursor = mkpart_align(cursor + sectors);
    }

    if (devio_read(id, 0, mbr, MKPART_MBR_SIZE) != MKPART_MBR_SIZE) {
        printf("mkpart: cannot read the master boot record of %s\n", id);
        return 1;
    }

    if (!force && mkpart_has_partitions(mbr)) {
        printf("mkpart: %s is already partitioned, use -f to overwrite\n", id);
        return 1;
    }

    /*
     * Only the table and the signature are replaced. What sits in front of them
     * is the boot code, which belongs to whoever installed it.
     */
    memset(mbr + MKPART_TABLE_OFFSET, 0, MKPART_MAX_PARTITIONS * 16);

    for (int index = 0; index < count; index++) {
        unsigned char* entry = mbr + MKPART_TABLE_OFFSET + index * 16;

        entry[0] = index + 1 == active ? MKPART_STATUS_ACTIVE : MKPART_STATUS_INACTIVE;
        mkpart_chs(entry + 1, partitions[index].start);
        entry[4] = MKPART_TYPE_LINUX;
        mkpart_chs(entry + 5, partitions[index].start + partitions[index].sectors - 1);
        mkpart_put32(entry + 8, partitions[index].start);
        mkpart_put32(entry + 12, partitions[index].sectors);

        printf("partition %d: %d sectors at %d%s\n", index + 1, (int) partitions[index].sectors,
               (int) partitions[index].start, index + 1 == active ? ", active" : "");
    }

    mbr[MKPART_SIGNATURE_OFFSET] = (unsigned char) (MKPART_SIGNATURE & 0xFF);
    mbr[MKPART_SIGNATURE_OFFSET + 1] = (unsigned char) (MKPART_SIGNATURE >> 8);

    if (devio_write(id, 0, mbr, MKPART_MBR_SIZE) != MKPART_MBR_SIZE) {
        printf("mkpart: cannot write the master boot record of %s\n", id);
        return 1;
    }

    memset(wipe, 0, sizeof(wipe));

    for (int index = 0; index < count; index++) {
        size_t offset = (size_t) partitions[index].start * storage.sector_size;

        if (devio_write(id, offset, wipe, sizeof(wipe)) != (int32_t) sizeof(wipe)) {
            printf("mkpart: cannot clear the start of partition %d\n", index + 1);
            return 1;
        }
    }

    int32_t volumes = devio_rescan(id);

    if (volumes < 0) {
        puts("mkpart: the table was written but the device could not be scanned again\n");
        return 1;
    }

    printf("%d volumes on %s\n", (int) volumes, id);

    return 0;
}
