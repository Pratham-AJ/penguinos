#include <stdint.h>
#include "heap.h"

static uint32_t heap_start;
static uint32_t heap_end;
static uint32_t heap_current;

void heap_init(uint32_t start, uint32_t size)
{
    heap_start   = start;
    heap_end     = start + size;
    heap_current = start;
}

void *kmalloc(uint32_t size)
{
    if (heap_current + size > heap_end) {
        return 0;   
    }
    void *addr = (void *)heap_current;
    heap_current += size;
    return addr;
}
