#include <err.h> 
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>

#include "log.h"

#include "nvme_spec.h"

/*
*   bar0_mapping: maps bar0 register into process userspace
*
*   The function return the pointer to bar0 on success, NULL
*   on error
*/
void * bar_map(char * resource_path, char * bdf)
{
    int32_t fd; // Utilisation de int32_t pour le descripteur
    void * bar_ptr;

    /* Open resource file for read/write access */
    fd = open(resource_path, O_RDWR | O_SYNC);
    if (fd < 0) {
        warn("Failed to open PCI resource file for %s", bdf);
        return NULL;
    }

    /* 
        Map BAR0 into userspace memory 
       MAP_SHARED so that NVMe device see modifications 
    */
    bar_ptr = mmap(NULL, NVME_BAR0_SIZE, PROT_READ | PROT_WRITE, 
        MAP_SHARED, fd, 0);
    
    if (bar_ptr == MAP_FAILED) {
        warn("Failed to mmap BAR0 for %s", bdf);
        close(fd);
        return NULL;
    }
    close(fd);
    return bar_ptr;
}


void bar_unmap(volatile void * bar)
{
    if (munmap((void*) bar, NVME_BAR0_SIZE) == -1) {
        warn("Error Failed to unmap bar, Consider possible leaks");
    }
}

/* 
*   ctrl_prop_print: print controller capability
*   to validate device initialization
*/
void nvme_capability_log(volatile void * bar)
{
    volatile Nvme_registers * regs = (volatile Nvme_registers *) bar;
    volatile Nvme_cap_prop * caps = (volatile Nvme_cap_prop*) &(regs->cap);

    printf("--- NVMe Controller Capabilities (CAP) ---\n");
    printf("MQES   : %u\n",   (uint16_t)caps->MQES);
    printf("CQR    : %u\n",   (uint8_t)caps->CQR);
    printf("AMS    : %u\n",   (uint8_t)caps->AMS);
    printf("TO     : %u\n",   (uint8_t)caps->TO);
    printf("DSTRD  : %u\n",   (uint8_t)caps->DSTRD);
    printf("CSS    : %u\n",   (uint8_t)caps->CSS);
    printf("MPSMIN : %u\n",   (uint8_t)caps->MPSMIN);
    printf("MPSMAX : %u\n",   (uint8_t)caps->MPSMAX);
    printf("PMRS   : %u\n",   (uint8_t)caps->PMRS);
    printf("CMBS   : %u\n",   (uint8_t)caps->CMBS);
    printf("------------------------------------------\n");
}

/* 
*   nvme_cmbloc_log: print controller buffer location
*   to validate device initialization
*/
void nvme_cmbloc_log(volatile void * bar) 
{
    volatile Nvme_registers * regs = (volatile Nvme_registers *) bar;
    volatile Nvme_cmbloc_prop * cmbloc = (volatile Nvme_cmbloc_prop*) &(regs->cmbloc);

    printf("--- NVMe Controller Capabilities (CAP) ---\n");
    printf("BIR     : %u\n",   (uint32_t) cmbloc->BIR);
    printf("CQMMS   : %u\n",   (uint32_t) cmbloc->CQMMS);
    printf("CQPDS   : %u\n",   (uint32_t) cmbloc->CQPDS);
    printf("CDPMLS  : %u\n",   (uint32_t) cmbloc->CDPMLS);
    printf("CDPCILS : %u\n",   (uint32_t) cmbloc->CDPCILS);
    printf("CDMMMS  : %u\n",   (uint32_t) cmbloc->CDMMMS);
    printf("CQDA    : %u\n",   (uint32_t) cmbloc->CQDA);
    printf("OFST    : %u\n",   (uint32_t) cmbloc->OFST);
    printf("------------------------------------------\n");

}



int8_t nvme_init(volatile void * bar)
{
    L_INFO("Inititialization of the NVMe controler");

    Nvme_registers * regs = (Nvme_registers *) bar;
    volatile Nvme_csts_prop * csts = (volatile Nvme_csts_prop*) &(regs->csts);

    int bsy_wait_tries = 0;

    while (!(csts->RDY && 1)) {
        /* 
        * Optimized busy waiting using ia32 pause mechanism 
        * generate the pause instruction that will delay the execution 
        * of the next instruction.
        */
        if (bsy_wait_tries < NVME_BUSY_WAIT_THRESHOLD) {
            __asm__ volatile ("pause");
        } else {
            /* 
             * If the device is too long to initialize then place the process
             * in sleep state to ensure driver do not take too much cpu ressources
             */
            usleep(NVME_BUSY_WAIT_THRESHOLD);
        }

        if (bsy_wait_tries > NVME_BUSY_WAIT_THRESHOLD*10) {
            L_ERR("Critical NVMe Hardware failure", 
                "bsy_wait threshold reached and driver appears broken");
            return EXIT_FAILURE;
        }
    }

    L_INFO("NVMe controler ready");

    L_INFO("Inititialization of the Admin submission queues");
    /* TODO : INIT queues */


    return EXIT_SUCCESS;
}