#include <err.h> 
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>

#include "macros.h"
#include "nvme_spec.h"
#include "nvme_sqe.h"
#include "nvme_queue_ctx.h"
#include "nvme_transport.h"

#include "nvme_core.h"


/* Global that define the number of active IO queue pair, start at 1, Admin queue takes 0*/
uint64_t IO_QUEUE_PAIR_NB = 1;

/*  bar_map: maps bar0 register into process userspace
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


/*  bar_unmap: unmap bar0 register from process userspace */
void bar_unmap(volatile void * bar)
{
    if (munmap((void*) bar, NVME_BAR0_SIZE) == -1) {
        warn("Error Failed to unmap bar, Consider possible leaks");
    }
}

int8_t nvme_enable(volatile Nvme_registers *regs)
{
    __asm__ volatile ("fence rw, rw" ::: "memory");

    int bsy_wait_tries = 0;

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

    return EXIT_SUCCESS;
}


int8_t nvme_init_handshake(volatile void * pci_bar, Nvmeq_context_t *admin_ctx) {

    L_INFO("Trying to init NVMe admin context and controller");

    printf("[OLD] Admin BAR virtual base: %p\n", pci_bar);

    /* Get NVMe registers to init the admin_ctx */
    volatile Nvme_registers * regs = (volatile Nvme_registers *) pci_bar;

    L_INFO("Initialization of the NVMe admin context");
    /* Initialize the contexts */
    if(nvme_init_queue_ctx(admin_ctx, regs, 0, 1) == EXIT_FAILURE) {
            L_ERR("Failed to initialize admin context", "nvme_init_queue_ctx failed");
            return EXIT_FAILURE;
    }

    L_INFO("Enabling NVMe controller");
    if (nvme_enable(regs)) {
        return EXIT_FAILURE;
    }

    nvme_cap_log(pci_bar);
    nvme_cc_log(pci_bar);
    nvme_csts_log(pci_bar);
    nvme_cmbloc_log(pci_bar);
    nvme_log_asq_acq(pci_bar);

    return EXIT_SUCCESS;
}


/* nvme_init_queue_ctx: sets up the memory and logic for a pair of NVMe queues
*
*  This function performs three main tasks:
*  1. Memory: Partitions the allocated buffer into SQ and CQ, and translates 
*  virtual addresses to physical addresses for DMA operations.
*  2. Transport: Resets host-side tracking indices (tail/head) and the 
*  Phase Tag bit to synchronization state.
*  3. Hardware: For Admin queues, it commits the physical addresses and 
*  queue depths directly to the Controller's BAR0 registers (ASQ/ACQ/AQA).
* 
*  Returns EXIT_SUCCESS on success, or EXIT_FAILURE if address translation 
*  or allocation fails.
*/
int8_t nvme_init_queue_ctx(Nvmeq_context_t *nvmeq_ctx, volatile Nvme_registers *regs, uint64_t qid, int8_t is_admin)
{
    uint8_t* sq_base = NULL;
    uint8_t* cq_base = NULL;

    /* nvmeq Context setup */
    sq_base = (uint8_t *) nvmeq_ctx->nvmeq_buff;
    cq_base = (uint8_t *) nvmeq_ctx->nvmeq_buff + PAGESIZE;

    if (nvmeq_ctx->nvmeq_buff == NULL) {
        L_ERR("nvmeq buffer is NULL", "Buffer allocation failed");
        return EXIT_FAILURE;
    }

    memset(sq_base, 0, PAGESIZE);
    memset(cq_base, 0, PAGESIZE);

    nvmeq_ctx->sq_virt_addr = (uint64_t) sq_base;
    nvmeq_ctx->cq_virt_addr = (uint64_t) cq_base;
    nvmeq_ctx->sq_phys_addr = nvmeq_to_phys(nvmeq_ctx, nvmeq_ctx->sq_virt_addr);
    nvmeq_ctx->cq_phys_addr = nvmeq_to_phys(nvmeq_ctx, nvmeq_ctx->cq_virt_addr);

    if (!nvmeq_ctx->sq_phys_addr || !nvmeq_ctx->cq_phys_addr) {
        L_ERR("Failed to initialize queues", "Address translation failure");
        return EXIT_FAILURE;
    }

    nvmeq_ctx->sq_tail = 0;
    nvmeq_ctx->sq_head = 0;
    nvmeq_ctx->cq_head = 0;
    nvmeq_ctx->expected_phase = 1;

    /* Get Doorbell Stride (CAP.DSTRD) from bits 32:35 and calculate stride in bytes: (4 << DSTRD) */
    uint32_t dstrd = (regs->cap >> 32) & 0xF;
    uint32_t step = 4 << dstrd;
    /* Calculate Doorbell offsets: 0x1000 is the base offset for all DBs in BAR0.
       Each qid has a pair of 32-bit registers (SQ Tail then CQ Head) spaced by stride.
    */
    nvmeq_ctx->sq_tdbl = (uint64_t)((uint8_t *)regs + 0x1000 + (2 * qid * step));
    nvmeq_ctx->cq_hdbl = (uintptr_t)regs + 0x1000 + (uintptr_t)((2 * qid + 1) * step);

    printf("[CRITICAL DEBUG] QID %lu: regs=%p, calculation result=%lx\n", 
        qid, (void*) regs, nvmeq_ctx->sq_tdbl);
    
    /* Queue configuration on controler */
    if (is_admin) {
        SET_NVME_REG_64(&(regs->asq), nvmeq_ctx->sq_phys_addr);
        SET_NVME_REG_64(&(regs->acq), nvmeq_ctx->cq_phys_addr);
        SET_NVME_PROP_FIELD_32(&(regs->aqa), Nvme_aqa_prop, ASQS, (PAGESIZE / SQ_ENTRY_SIZE) - 1);
        SET_NVME_PROP_FIELD_32(&(regs->aqa), Nvme_aqa_prop, ACQS, (PAGESIZE / CQ_ENTRY_SIZE) - 1);
        L_INFO("Admin Queue configured in registers");
    } else {

        L_INFO("I/O Queue buffer prepared, pending Admin registration command");
    }

    return EXIT_SUCCESS;
}

int8_t nvme_ioqueue_create(volatile void * bar, Nvmeq_context_t *admin, Nvmeq_context_t * io, uint16_t qid) 
{
    /* Create the proper admin commands */
    volatile Nvme_registers * regs = (volatile Nvme_registers *) bar;

    /* STEP 2 : Set queue pair sizes */
    SET_NVME_PROP_FIELD_32(&(regs->cc), Nvme_cc_prop, IOSQES, NVME_LOG2_SQ_SIZE);
    SET_NVME_PROP_FIELD_32(&(regs->cc), Nvme_cc_prop, IOCQES, NVME_LOG2_CQ_SIZE);
    
    L_INFO("Sending I/O completion creation request");
    if (admin_send(bar, 
                        nvme_create_iocompcreate_sqe(
                                        qid, 
                                        io->sq_depth, 
                                        io->cq_phys_addr),
                        admin,
                        0))
        return EXIT_FAILURE;
    L_SUCC("Success");

    L_INFO("Sending I/O submission creation request");
    if (admin_send(bar, 
                        nvme_create_iosubcreate_sqe(
                                    qid, 
                                    io->sq_depth, 
                                    io->sq_phys_addr, 
                                    IO_QUEUE_PAIR_NB), 
                        admin, 0))
        return EXIT_FAILURE;

    L_SUCC("Success");
    /* If confirmend created */
    IO_QUEUE_PAIR_NB++;
    return EXIT_SUCCESS;
}



/* 
*   ctrl_prop_print: print controller capability
*   to validate device initialization
*/
void nvme_cap_log(volatile void * bar)
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