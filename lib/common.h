#ifndef COMMON_H
#define COMMON_H



#include "stdint.h"
#include "stddef.h"


#define UNUSED(x)   ((void)(x))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static inline uint32_t align_up(uint32_t val, uint32_t align)
{
    return (val + align - 1) & ~(align - 1);
}

#endif 
