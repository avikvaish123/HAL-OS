#include "paging.h"
#include "scheduler.h"
#include "system_calls.h"

//align PDs and PTs, don't map anything after 8 MB


/* void init_paging(void)
 * Inputs:      void
 * Return Value: void
 * Function: initializes paging by translating video memory to directory 0, page 184 and keeping kernel data in 4MB-8MB.  Also loads page directory address and enables paging. */
void 
init_paging() {
    //iterate over all page directory entries and set present bit to 0 for each, everything else is Dont cares
    int i;
    for(i = 0; i < PAGE_VEC; i++)
    {
        // This sets the following flags to the directory entries:
        //   Not Present: The directory entry is not present
        //  All other values are unimportant during initialization
        directoryArray[i].present=0x0;
        //  one value set to differ from other initialized pages
        directoryArray[i].readWrite=0x1;
        directoryArray[i].userSupervisor=0x0;
        directoryArray[i].writeThrough=0x0;
        directoryArray[i].cacheDisable=0x0;
        directoryArray[i].accessed=0x0;
        directoryArray[i].avl=0x0;
        directoryArray[i].pageSize=0x0;
        directoryArray[i].available=0x0;
        directoryArray[i].offset_31_12=0x0;
    }

    //manually enable first directory entry for purpose of video memory translation
    // Set directory to present
    directoryArray[0].present=0x1;
    //enable read/write
    directoryArray[0].readWrite=0x1;
    //give user access to page
    directoryArray[0].userSupervisor=0x0;
    //not sure
    directoryArray[0].writeThrough=0x0;
    directoryArray[0].cacheDisable=0x0;
    directoryArray[0].accessed=0x0;
    directoryArray[0].avl=0x0;
    directoryArray[0].pageSize=0x0;
    directoryArray[0].available=0x0;
    //set address to point to page table array
    directoryArray[0].offset_31_12 = (uint32_t)tableArray >> PAGING_OFFSET;

    //iterate over all page table entries and set present bit to 0 for each, everything else is Dont cares
    for(i = 0; i < PAGE_VEC; i++)
    {
        // This sets the following flags to the directory entries:
        //   Not Present: The directory entry is not present
        //  All other values are unimportant during initialization
        tableArray[i].present=0x0;
        //  one value set to differ from other initialized pages
        tableArray[i].readWrite=0x1;
        tableArray[i].userSupervisor=0x0;
        tableArray[i].writeThrough=0x0;
        tableArray[i].cacheDisable=0x0;
        tableArray[i].accessed=0x0;
        tableArray[i].dirty=0x0;
        tableArray[i].pageAttributeTable=0x0;
        tableArray[i].global=0x0;
        tableArray[i].available=0x0;
        tableArray[i].offset_31_12=0x0;
    }

    //manually enable 184th page for video memory
    //enable present bit so memory is accessible
    tableArray[VIDEO_MEMORY_PAGE].present=0x1;
    //enable read/write to change video memory
    tableArray[VIDEO_MEMORY_PAGE].readWrite=0x1;
    //enable user to access page
    tableArray[VIDEO_MEMORY_PAGE].userSupervisor=0x0;
    tableArray[VIDEO_MEMORY_PAGE].writeThrough=0x0;
    tableArray[VIDEO_MEMORY_PAGE].cacheDisable=0x0;
    tableArray[VIDEO_MEMORY_PAGE].accessed=0x0;
    tableArray[VIDEO_MEMORY_PAGE].dirty=0x0;
    tableArray[VIDEO_MEMORY_PAGE].pageAttributeTable=0x0;
    tableArray[VIDEO_MEMORY_PAGE].global=0x0;
    tableArray[VIDEO_MEMORY_PAGE].available=0x0;
    //set page# to physical video memory location offset by 12 bits 
    tableArray[VIDEO_MEMORY_PAGE].offset_31_12=VIDEO >> PAGING_OFFSET;

    //enable the terminal video pages
    for (i = 0; i < MAX_TERMINALS; i++) {
        tableArray[TERMINAL_VID_PAGE + i].present=0x1;
        tableArray[TERMINAL_VID_PAGE + i].readWrite=0x1;
        tableArray[TERMINAL_VID_PAGE + i].userSupervisor=0x0;
        tableArray[TERMINAL_VID_PAGE + i].writeThrough=0x0;
        tableArray[TERMINAL_VID_PAGE + i].cacheDisable=0x0;
        tableArray[TERMINAL_VID_PAGE + i].accessed=0x0;
        tableArray[TERMINAL_VID_PAGE + i].dirty=0x0;
        tableArray[TERMINAL_VID_PAGE + i].pageAttributeTable=0x0;
        tableArray[TERMINAL_VID_PAGE + i].global=0x0;
        tableArray[TERMINAL_VID_PAGE + i].available=0x0;
        tableArray[TERMINAL_VID_PAGE + i].offset_31_12 = (TERMINAL_VIDEO + i * ALIGN_4KB) >> PAGING_OFFSET;
    }

    //manually enable 2nd directory for purpose of kernel memory
    directoryArray[1].present=0x1;
    //enable both for now, present needed
    directoryArray[1].readWrite=0x1;
    //not sure whether to give permission or not, enabled for now
    directoryArray[1].userSupervisor=0x0;
    directoryArray[1].writeThrough=0x0;
    directoryArray[1].cacheDisable=0x0;
    directoryArray[1].accessed=0x0;
    //dirty: 1 bit
    //pageSize: 1 bit
    //global: 1 bit
    //avail:3 bits
    directoryArray[1].avl=0x1;  //1  gets the dirty bit
    directoryArray[1].pageSize=0x1;  //1 get the page size bit
    directoryArray[1].available=0x0;  //4 gets the global and avail bits
    //points page number to the 1024th page, or start of 4MB
    directoryArray[1].offset_31_12=DIR1_PAGE;

    // create the user program page
    directoryArray[USER_SPACE_DIR_NUM].present = 0x1;
    directoryArray[USER_SPACE_DIR_NUM].readWrite = 0x1;
    directoryArray[USER_SPACE_DIR_NUM].userSupervisor = 0x1;
    directoryArray[USER_SPACE_DIR_NUM].writeThrough = 0x0;
    directoryArray[USER_SPACE_DIR_NUM].cacheDisable = 0x0;
    directoryArray[USER_SPACE_DIR_NUM].accessed = 0x0;
    directoryArray[USER_SPACE_DIR_NUM].avl = 0x0;
    directoryArray[USER_SPACE_DIR_NUM].pageSize = 0x1;
    directoryArray[USER_SPACE_DIR_NUM].available = 0x0;
    directoryArray[USER_SPACE_DIR_NUM].offset_31_12 = get_phys_addr(0); // sets program image to PID 0 on initialization

    // manually set the video memory page
    directoryArray[VIDMAP_VMEM_LOC].present = 0x1;
    directoryArray[VIDMAP_VMEM_LOC].readWrite = 0x1;
    directoryArray[VIDMAP_VMEM_LOC].userSupervisor = 0x1;
    directoryArray[VIDMAP_VMEM_LOC].writeThrough = 0x0;
    directoryArray[VIDMAP_VMEM_LOC].pageSize = 0x0;
    directoryArray[VIDMAP_VMEM_LOC].offset_31_12 = (uint32_t)vmemTableArray >> PAGING_OFFSET;

    //iterate over all page table entries and set present bit to 0 for each, everything else is Dont cares
    for(i = 0; i < PAGE_VEC; i++)
    {
        // This sets the following flags to the directory entries:
        //   Not Present: The directory entry is not present
        //  All other values are unimportant during initialization
        vmemTableArray[i].present=0x0;
        //  one value set to differ from other initialized pages
        vmemTableArray[i].readWrite=0x1;
        vmemTableArray[i].userSupervisor=0x0;
        vmemTableArray[i].writeThrough=0x0;
        vmemTableArray[i].cacheDisable=0x0;
        vmemTableArray[i].accessed=0x0;
        vmemTableArray[i].dirty=0x0;
        vmemTableArray[i].pageAttributeTable=0x0;
        vmemTableArray[i].global=0x0;
        vmemTableArray[i].available=0x0;
        vmemTableArray[i].offset_31_12=0x0;
    }

    // setup page to map
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].present=0x1;
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].readWrite=0x1;
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].userSupervisor=0x1;
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].writeThrough=0x0;
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].cacheDisable=0x0;
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].accessed=0x0;
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].dirty=0x0;
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].pageAttributeTable=0x0;
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].global=0x0;
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].available=0x0;
    vmemTableArray[VIDMAP_VMEM_LOC_PAGE].offset_31_12 = VIDEO >> PAGING_OFFSET;

    //load directory to CR3 register to set up for paging
    loadPageDirectory((uint32_t *) directoryArray);

    //enable paging through cr0 register and enable extended page size too
    enablePaging();
}


