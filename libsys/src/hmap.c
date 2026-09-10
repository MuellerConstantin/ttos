#include <hmap.h>
#include <syscall.h>

void* hmap_alloc(size_t n_pages) {
    return (void*) syscall1(SYSCALL_ALLOC_HEAP, n_pages);
}
