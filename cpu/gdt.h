#ifndef GDT_H
#define GDT_H

// Structure for a GDT entry
struct gdt_entry {
    unsigned short limit_low;  // Lower 16 bits of the limit
    unsigned short base_low;   // Lower 16 bits of the base
    unsigned char base_middle;  // Next 8 bits of the base
    unsigned char access;       // Access flags
    unsigned char granularity;  // Granularity flags
    unsigned char base_high;    // Last 8 bits of the base
};

// Structure for GDT pointer
struct gdt_ptr {
    unsigned short limit;       // Limit of GDT
    unsigned int base;          // Base address of GDT
};

// Function to initialize the GDT
void gdt_init();

#endif // GDT_H
