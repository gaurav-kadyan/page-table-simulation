#ifndef CONFIG_H
#define CONFIG_H

#define PAGE_SIZE 4096 //page size in bytes
#define PHYSICAL_TOTAL_FRAMES 1048576 // number of pages in physical memory
#define VIRTUAL_MEMORY_PAGES 1048576 // number of pages in virtual memory
#define PAGE_TABLE_ENTERY_SIZE 32 // size in bits

// Define the starting addresses for different sections
const uint64_t START_MEMORY_SECTION = 0x00000000;
const uint64_t START_TEXT_SECTION = 0x00000000;
const uint64_t START_DATA_SECTION = 0x20000000;
const uint64_t START_STACK_SECTION = 0x30000000;
const uint64_t START_HEAP_SECTION = 0x50000000;
const uint64_t END_MEMORY_SECTION = 0xfffff000;
#endif