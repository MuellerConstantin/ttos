#ifndef _LIBK_UUID_H
#define _LIBK_UUID_H

#include <stdint.h>
#include <stdlib.h>

typedef struct uuid uuid_t;

struct uuid {
    uint8_t bytes[16];
};

/**
 * Generates a UUIDv4.
 * 
 * @param uuid The UUID to store the generated UUID in.
 */
void generate_uuid_v4(uuid_t* uuid);

/**
 * Converts a UUID to a string.
 * 
 * @param uuid The UUID to convert.
 * @param buffer The buffer to store the string in. Must be at least 37 bytes long.
 */
void uuid_v4_to_string(uuid_t* uuid, char* buffer);

/**
 * Compares two UUIDs.
 * 
 * @param uuid1 The first UUID.
 * @param uuid2 The second UUID.
 * @return 0 if the UUIDs are equal, a negative number if uuid1 is less than uuid2, a positive number if uuid1 is greater than uuid2.
 */
int uuid_v4_compare(uuid_t* uuid1, uuid_t* uuid2);

#endif // _LIBK_UUID_H
