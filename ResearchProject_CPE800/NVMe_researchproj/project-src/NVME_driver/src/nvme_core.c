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
#include "nvme_q.h"

#include "nvme_core.h"

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
    Nvme_cap_prop caps = (Nvme_cap_prop) regs->cap;

    printf("--- NVMe Controller Capabilities (CAP) ---\n");
    printf("MQES   : %u\n",   (uint16_t)caps.fields.MQES);
    printf("CQR    : %u\n",   (uint8_t)caps.fields.CQR);
    printf("AMS    : %u\n",   (uint8_t)caps.fields.AMS);
    printf("TO     : %u\n",   (uint8_t)caps.fields.TO);
    printf("DSTRD  : %u\n",   (uint8_t)caps.fields.DSTRD);
    printf("CSS    : %u\n",   (uint8_t)caps.fields.CSS);
    printf("MPSMIN : %u\n",   (uint8_t)caps.fields.MPSMIN);
    printf("MPSMAX : %u\n",   (uint8_t)caps.fields.MPSMAX);
    printf("PMRS   : %u\n",   (uint8_t)caps.fields.PMRS);
    printf("CMBS   : %u\n",   (uint8_t)caps.fields.CMBS);
    printf("------------------------------------------\n");
}

/* * nvme_cc_log: print controller configuration
* Reference: NVM Express® Base Specification, Revision 2.2, Section 3.1.3
*/
void nvme_cc_log(volatile void * bar) 
{
    volatile Nvme_registers * regs = (volatile Nvme_registers *) bar;
    Nvme_cc_prop cc = (Nvme_cc_prop) regs->cc;

    printf("--- NVMe Controller Configuration (CC) ---\n");
    printf("EN      : %u\n", cc.fields.EN);
    printf("CSS     : %u\n", cc.fields.CSS);
    printf("MPS     : %u\n", cc.fields.MPS);
    printf("ARS     : %u\n", cc.fields.ARS);
    printf("SHN     : %u\n", cc.fields.SHN);
    printf("IOSQES  : %u\n", cc.fields.IOSQES);
    printf("IOCQES  : %u\n", cc.fields.IOCQES);
    printf("------------------------------------------\n");
}

/* * nvme_aqa_log: print admin queue attributes
* Reference: NVM Express® Base Specification, Revision 2.2, Section 3.1.5
*/
void nvme_csts_log(volatile void * bar) 
{
    volatile Nvme_registers * regs = (volatile Nvme_registers *) bar;
    
    Nvme_csts_prop csts = (Nvme_csts_prop) regs->csts;

    printf("--- NVMe Controller Status (CSTS) ---\n");
    printf("RDY     : %u\n", csts.fields.RDY);
    printf("CFS     : %u\n", csts.fields.CFS);
    printf("SHST    : %u\n", csts.fields.SHST);
    printf("NSSRO   : %u\n", csts.fields.NSSRO);
    printf("PP      : %u\n", csts.fields.PP);
    printf("ST      : %u\n", csts.fields.ST);
    printf("-------------------------------------\n");
}

/* 
*   nvme_cmbloc_log: print controller buffer location
*   to validate device initialization
*/
void nvme_cmbloc_log(volatile void * bar) 
{
    volatile Nvme_registers * regs = (volatile Nvme_registers *) bar;

    Nvme_cmbloc_prop cmbloc = (Nvme_cmbloc_prop) regs->cmbloc;

    printf("--- NVMe Controller memory buffer location  (CMBLOC) ---\n");
    printf("BIR     : %u\n",   (uint32_t) cmbloc.fields.BIR);
    printf("CQMMS   : %u\n",   (uint32_t) cmbloc.fields.CQMMS);
    printf("CQPDS   : %u\n",   (uint32_t) cmbloc.fields.CQPDS);
    printf("CDPMLS  : %u\n",   (uint32_t) cmbloc.fields.CDPMLS);
    printf("CDPCILS : %u\n",   (uint32_t) cmbloc.fields.CDPCILS);
    printf("CDMMMS  : %u\n",   (uint32_t) cmbloc.fields.CDMMMS);
    printf("CQDA    : %u\n",   (uint32_t) cmbloc.fields.CQDA);
    printf("OFST    : %u\n",   (uint32_t) cmbloc.fields.OFST);
    printf("---------------------------------------------------------\n");

}

void nvme_log_asq_acq(volatile void * bar) 
{
    volatile Nvme_registers * regs = (volatile Nvme_registers *) bar;

    printf("--- NVMe ASQ and ACQ address---\n");
    printf("Reg ASQ : %lx\n",   (uint64_t) regs->asq);
    printf("Reg ACQ : %lx\n",   (uint64_t) regs->acq);
    printf("-------------------------------\n");

}

/* 
* nvme_aqa_log: print admin queue attributes
* Reference: NVM Express® Base Specification, Revision 2.2, Section 3.1.5
*/
void nvme_aqa_log(volatile void * bar) 
{
    volatile Nvme_registers * regs = (volatile Nvme_registers *) bar;
    volatile Nvme_aqa_prop aqa = (Nvme_aqa_prop) regs->aqa;

    printf("--- NVMe Admin Queue Attributes (AQA) ---\n");
    printf("ASQS    : %u\n", aqa.fields.ASQS);
    printf("ACQS    : %u\n", aqa.fields.ACQS);
    printf("------------------------------------------\n");
}

/* 
* nvme_init: init the nvme controler : create queues into previous nvmeq_ctx
* and update registers to specify physical address and size
* of queues to controller, Then enable the controller and wait for it to be ready
*/
int8_t nvme_init(volatile void * bar, Nvmeq_context_t * nvmeq_ctx)
{
    L_INFO("Initialization of the NVMe controller");

    /* Accès à la structure des registres */
    volatile Nvme_registers * regs = (volatile Nvme_registers *) bar;

    int bsy_wait_tries = 0;
    uint8_t* sq_base = NULL;
    uint8_t* cq_base = NULL;
    uint64_t sq_phys_addr = 0;
    uint64_t cq_phys_addr = 0;

    /* nvmeq Context setup */
    sq_base = (uint8_t *) nvmeq_ctx->nvmeq_buff;
    cq_base = (uint8_t *) nvmeq_ctx->nvmeq_buff + PAGESIZE;

    if (nvmeq_ctx->nvmeq_buff == NULL) {
        L_ERR("nvmeq buffer is NULL", "Buffer allocation failed");
        return EXIT_FAILURE;
    }

    memset(sq_base, 0, PAGESIZE);
    memset(cq_base, 0, PAGESIZE);

    sq_phys_addr = nvmeq_to_phys(nvmeq_ctx, (uint64_t) sq_base);
    cq_phys_addr = nvmeq_to_phys(nvmeq_ctx, (uint64_t) cq_base);

    if (sq_phys_addr == EXIT_FAILURE || cq_phys_addr == EXIT_FAILURE) {
        L_ERR("Failed to initialize queues", "Address translation failure");
        return EXIT_FAILURE;
    }

    /* Queue configuration on controler */
    SET_NVME_REG_64(&(regs->asq), sq_phys_addr);
    SET_NVME_REG_64(&(regs->acq), cq_phys_addr);

    SET_NVME_PROP_FIELD_32(&(regs->aqa), Nvme_aqa_prop, ASQS, (PAGESIZE / SQ_ENTRY_SIZE) - 1);
    SET_NVME_PROP_FIELD_32(&(regs->aqa), Nvme_aqa_prop, ACQS, (PAGESIZE / CQ_ENTRY_SIZE) - 1);

    /* Enable controler */
    SET_NVME_PROP_FIELD_32(&(regs->cc), Nvme_cc_prop, EN, 1);

    L_SUCC("NVMe controller enabled");

    /* Polling on the controller */
    while (1) {
        /* Retreive csts register as raw */
        uint32_t csts_raw = ((Nvme_csts_prop) regs->csts).raw;
        
        /* Memory barrier to wait for the read to be completed before any next reads */
        __asm__ volatile ("fence r, r" ::: "memory");

        Nvme_csts_prop csts;
        csts.raw = csts_raw;

        if (csts.fields.RDY == 1) {
            break;
        }

        if (bsy_wait_tries < NVME_BUSY_WAIT_THRESHOLD) {
            __asm__ volatile ("pause");
        } else {
            usleep(1000);
        }

        bsy_wait_tries++;
        if (bsy_wait_tries > NVME_BUSY_WAIT_THRESHOLD * 10) {
            L_ERR("Critical NVMe Hardware failure", "Controller failed to be ready");
            return EXIT_FAILURE;
        }
    }
    L_INFO("NVMe controller ready");

    nvmeq_ctx->sq_phys_addr = sq_phys_addr;
    nvmeq_ctx->cq_phys_addr = cq_phys_addr;
    nvmeq_ctx->sq_virt_addr = (uint64_t) sq_base;
    nvmeq_ctx->cq_virt_addr = (uint64_t) cq_base;

    nvme_capability_log(bar);
    nvme_cc_log(bar);
    nvme_csts_log(bar);
    nvme_cmbloc_log(bar);
    nvme_aqa_log(bar);
    nvme_log_asq_acq(bar);

    return EXIT_SUCCESS;
}