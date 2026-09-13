#include <devio.h>
#include <volio.h>
#include <stdio.h>
#include <string.h>
#include <termio.h>

/** Bytes shown per line, the usual width for a hex dump. */
#define DEVDUMP_LINE_BYTES 16

/** Bytes read per call, and the default amount dumped when none is given. */
#define DEVDUMP_CHUNK 512

/** Room for the widest line the dump can produce. */
#define DEVDUMP_LINE_LENGTH 128

static unsigned char buffer[DEVDUMP_CHUNK];

/*
 * Reads a decimal or, with a 0x prefix, a hexadecimal number. Returns 0 when
 * the text is not a number at all, which the caller reports as usage.
 */
static int devdump_number(const char* text, size_t* out) {
    size_t value = 0;
    size_t base = 10;
    int digits = 0;

    if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
        text += 2;
    }

    for (; *text; text++) {
        size_t digit;

        if (*text >= '0' && *text <= '9') {
            digit = (size_t) (*text - '0');
        } else if (base == 16 && *text >= 'a' && *text <= 'f') {
            digit = (size_t) (*text - 'a') + 10;
        } else if (base == 16 && *text >= 'A' && *text <= 'F') {
            digit = (size_t) (*text - 'A') + 10;
        } else {
            return 0;
        }

        value = value * base + digit;
        digits++;
    }

    *out = value;

    return digits > 0;
}

/*
 * Writes a number as exactly `digits` hexadecimal characters. printf is no use
 * here: its %x always prefixes 0x and counts that prefix towards the field
 * width, which leaves the columns of a dump ragged.
 */
static char* devdump_hex(char* cursor, size_t value, int digits) {
    static const char alphabet[] = "0123456789abcdef";

    for (int shift = (digits - 1) * 4; shift >= 0; shift -= 4) {
        *cursor++ = alphabet[(value >> shift) & 0xF];
    }

    return cursor;
}

/** Writes one line of the dump: offset, the bytes in hex, then their printable form. */
static void devdump_line(char* line, size_t offset, const unsigned char* bytes, size_t length) {
    char* cursor = line;

    cursor = devdump_hex(cursor, offset, 8);
    *cursor++ = ' ';
    *cursor++ = ' ';

    for (size_t index = 0; index < DEVDUMP_LINE_BYTES; index++) {
        if (index < length) {
            cursor = devdump_hex(cursor, bytes[index], 2);
        } else {
            *cursor++ = ' ';
            *cursor++ = ' ';
        }

        *cursor++ = ' ';

        // A gap in the middle keeps the columns countable.
        if (index == DEVDUMP_LINE_BYTES / 2 - 1) {
            *cursor++ = ' ';
        }
    }

    *cursor++ = ' ';
    *cursor++ = '|';

    for (size_t index = 0; index < length; index++) {
        *cursor++ = (bytes[index] >= 0x20 && bytes[index] < 0x7F) ? (char) bytes[index] : '.';
    }

    *cursor++ = '|';
    *cursor++ = '\n';
    *cursor = '\0';
}

int main(int argc, char** argv) {
    int argument = 1;
    int volume = 0;

    if (argument < argc && strcmp(argv[argument], "-v") == 0) {
        volume = 1;
        argument++;
    }

    int count = argc - argument;

    if (count < 2 || count > 3) {
        puts("usage: devdump [-v] <device> <offset> [length]\n");
        puts("       -v dumps a volume instead of a device, from its own start\n");
        return 1;
    }

    const char* id = argv[argument++];
    size_t offset;
    size_t length = DEVDUMP_CHUNK;

    if (!devdump_number(argv[argument], &offset) || (count == 3 && !devdump_number(argv[argument + 1], &length))) {
        puts("devdump: offset and length have to be numbers, 0x for hex\n");
        return 1;
    }

    termio_pager_t pager;
    char line[DEVDUMP_LINE_LENGTH];

    termio_pager_init(&pager);

    while (length > 0) {
        size_t chunk = length < DEVDUMP_CHUNK ? length : DEVDUMP_CHUNK;
        int32_t read = volume ? volio_read(id, offset, buffer, chunk) : devio_read(id, offset, buffer, chunk);

        if (read < 0) {
            puts(volume ? "devdump: cannot read from volume\n" : "devdump: cannot read from device\n");
            return 1;
        }

        // A short read is the end of the device.
        if (read == 0) {
            break;
        }

        for (int32_t index = 0; index < read; index += DEVDUMP_LINE_BYTES) {
            size_t rest = (size_t) (read - index);

            devdump_line(line, offset + (size_t) index, buffer + index, rest < DEVDUMP_LINE_BYTES ? rest : DEVDUMP_LINE_BYTES);

            // The reader has seen enough, the rest of the device is not worth reading.
            if (termio_pager_puts(&pager, line) < 0) {
                return 0;
            }
        }

        offset += (size_t) read;
        length -= (size_t) read;
    }

    return 0;
}
