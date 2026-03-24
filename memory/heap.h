#ifndef HEAP_H
#define HEAP_H



#include <stdint.h>

void  heap_init(uint32_t start, uint32_t size);
void *kmalloc(uint32_t size);

#endif 