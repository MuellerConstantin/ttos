#ifndef _KERNEL_UTIL_SHORTID_H
#define _KERNEL_UTIL_SHORTID_H

#include <stdbool.h>
#include <stddef.h>

// Length of a short identifier (in hex characters, excluding the terminating
// null). Short enough to type by hand.
#define SHORT_ID_LENGTH 6

/**
 * Generates a random short hex identifier.
 *
 * If an exists predicate is given, the id is regenerated until the predicate
 * returns false, guaranteeing uniqueness by construction against whatever set
 * the predicate checks. Pass NULL to skip the uniqueness check.
 *
 * @param buffer The buffer to write the id to. Must be at least SHORT_ID_LENGTH + 1 bytes.
 * @param exists Optional predicate returning whether an id is already taken, or NULL.
 */
void generate_short_id(char* buffer, bool (*exists)(const char* id));

#endif // _KERNEL_UTIL_SHORTID_H
