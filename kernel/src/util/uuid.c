#include <util/uuid.h>

void generate_uuid_v4(uuid_t* uuid) {
    for (int i = 0; i < 16; ++i) {
        uuid->bytes[i] = (uint8_t)(random_next() & 0xFF);
    }

    // Sets the version (4 for UUIDv4) -> Byte 6 must be 0100xxxx
    uuid->bytes[6] = (uuid->bytes[6] & 0x0F) | 0x40;

    // Set the variant (for RFC 4122) -> Byte 8 must be 10xxxxxx
    uuid->bytes[8] = (uuid->bytes[8] & 0x3F) | 0x80;
}

void uuid_v4_to_string(uuid_t* uuid, char* buffer) {
    const char* hex_digits = "0123456789abcdef";
    int pos = 0;

    for (int i = 0; i < 16; ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            buffer[pos++] = '-';
        }

        buffer[pos++] = hex_digits[(uuid->bytes[i] >> 4) & 0x0F];
        buffer[pos++] = hex_digits[uuid->bytes[i] & 0x0F];
    }

    buffer[pos] = '\0';
}

int uuid_v4_compare(uuid_t* uuid1, uuid_t* uuid2) {
    for (int i = 0; i < 16; ++i) {
        if (uuid1->bytes[i] < uuid2->bytes[i]) {
            return -1;
        } else if (uuid1->bytes[i] > uuid2->bytes[i]) {
            return 1;
        }
    }

    return 0;
}
