#ifndef _LIBSYS_HMAP_H
#define _LIBSYS_HMAP_H

#include <stddef.h>

/**
 * Increases the heap size by the specified number of pages.
 * 
 * @param n_pages The number of pages to increase the heap size by.
 * @return Returns the new heap's limit, or NULL if the heap could not be increased.
 */
void* hmap_alloc(size_t n_pages);

#endif // _LIBSYS_HMAP_H
