
#ifndef _PAGING_H
#define _PAGING_H

#ifndef ASM

#include "types.h"
#include "lib.h"
#include "x86_desc.h"
#include "lib.h"

/* Number of vectors in the page directory/table */
#define PAGE_VEC     1024
/* Size of bytes to align to */
#define ALIGN_4KB    4096
/* Number of offset bits in page entries to reference page location, 20 bits with 12 more bits afterwards assumed to be 0*/
#define PAGE_ADDR_B  20
/* offset address by 12 bits to get the page #, add in the 12 bits to get direct page address*/
#define PAGING_OFFSET 12
/* page number of video memory, translated from 0xB8000 in lib.c*/
#define VIDEO_MEMORY_PAGE 184
/* page number for first terminal vid mem*/
#define TERMINAL_VID_PAGE 186
/* locations for first terminal video mem*/
#define TERMINAL_VIDEO  0xBA000
/* page number for 4MB page pointed to by 2nd directory entry*/
#define DIR1_PAGE 0x400
/* taken from lib.c */
#define VIDEO       0xB8000

#define VIDMAP_VMEM_LOC         35
#define VIDMAP_VMEM_LOC_PAGE    10

/* page directory entry struct, defined for 32 bits, set using variable.field= 0xHEX */
typedef union pdir_entry {
    uint32_t val;
    struct {
        // present = 1 specifies that page is mapped to physical memory at the moment, 0 specifies that page isn't mapped to physical memory or that mapped addresses have swapped out to disk
        uint32_t present: 1;

        // read/write permissions flag
        uint32_t readWrite: 1;

        // control access to page based on privilege level, if bit is set to 1 then anyone can access, if set to 0 than only supervisor can access
        uint32_t userSupervisor: 1;

        // enables write-through caching, if set to 0 then write-back is enabled instead
        uint32_t writeThrough: 1;

        // if set to 1, page will not be cached
        uint32_t cacheDisable: 1;

        //indicates whether a PDE or PTE was read during virtual address translation
        uint32_t accessed: 1;

        //set to 0
        uint32_t avl: 1;

        //1 indicates a 4 MiB page size and 0 indicates a 4 KiB page size
        uint32_t pageSize: 1;   

        // don't cares, value not used by the processor, can use for later tracking or record keeping if necessary
        uint32_t available: 4;

        // specificies 20 msb of address of page, 12 lsb are assumed to be 0 since each page should be aligned by 4 KB
        uint32_t offset_31_12 : PAGE_ADDR_B;
    } __attribute__ ((packed));
} pdir_entry_t;


/* page table entry struct, defined for 32 bits, set using variable.field= 0xHEX */
typedef union ptble_entry {
    uint32_t val;
    struct {
        // present = 1 specifies that page is mapped to physical memory at the moment, 0 specifies that page isn't mapped to physical memory or that mapped addresses have swapped out to disk       
        uint32_t present: 1;

        // read/write permissions flag
        uint32_t readWrite: 1;

        // control access to page based on privilege level, if bit is set to 1 then anyone can access, if set to 0 than only supervisor can access
        uint32_t userSupervisor: 1;

        // enables write-through caching, if set to 0 then write-back is enabled instead
        uint32_t writeThrough: 1;

        // if set to 1, page will not be cached
        uint32_t cacheDisable: 1;

        //indicates whether a PDE or PTE was read during virtual address translation
        uint32_t accessed: 1;

        //set to 1 to specify a page has been written to
        uint32_t dirty: 1;

        //set to 1 if PAT is supported, PAT along with PCD and PWD determines memory caching type
        uint32_t pageAttributeTable: 1;

        //set to 1 to tell processor not to invalidate TLB entry corresponding to the page on a MOV to CR3 instruction, PGE in CR4 must be set to enable global pages
        uint32_t global: 1;

        // don't cares, value not used by the processor, can use for later tracking or record keeping if necessary
        uint32_t available: 3;

        // specificies 20 msb of address of page, 12 lsb are assumed to be 0 since each page should be aligned by 4 KB
        uint32_t offset_31_12 : PAGE_ADDR_B;
    } __attribute__ ((packed));
} ptble_entry_t;



/* 2nd level of indirection: contains 1024 4KB-aligned page directory entries that either point to 4KB page tables, 4 MB page, or nothing*/
pdir_entry_t directoryArray[PAGE_VEC] __attribute__((aligned (ALIGN_4KB)));
/* 1st level of indirection: contains 1024 4KB-aligned page table entires that either point to 4KB pages or nothing */
ptble_entry_t tableArray[PAGE_VEC] __attribute__((aligned (ALIGN_4KB)));

ptble_entry_t vmemTableArray[PAGE_VEC] __attribute__((aligned (ALIGN_4KB)));

/* initializes paging by translating video memory to directory 0, page 184 and keeping kernel data in 4MB-8MB.  Also loads page directory address and enables paging. */
void init_paging();

#endif /* ASM */
#endif /* _PAGING_H */

