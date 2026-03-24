#ifndef NVME_TRANSPORT_H
#define NVME_TRANSPORT_H

#include <stdint.h>

#include "nvme_spec.h"
#include "nvme_queue_context.h"

/* NVME QUEUES ENTRIES FORMAT */

/*
* Nvme_sqe_t : Definition of the Submission Queue Entry (SQE)
* Format : 64 bytes total
* (NVM Express® Base Specification, Revision 2.2, p91-94)
*/
typedef struct nvme_sqe_t {
    /* DWORD 0 (4 byte) Command Dword 0 (CDW0) */
    uint32_t opcode : 8;     /* Opcode de la commande */
    uint32_t fuse   : 2;     /* Fuse operations */
    uint32_t res0   : 4;     /* Reserved */
    uint32_t psdt   : 2;     /* PRP or SGL for Data Transfer */
    uint32_t cid    : 16;    /* Command Identifier */

    /* DWORD 1 : Namespace Identifier used in certain commands */
    uint32_t nsid;    /* Namespace Identifier (NSID) */

    /* DWORD 2 & 3 : theses are command specifc */
    uint32_t dword2;    /* Command Specific Dword 2 */
    uint32_t dword3;    /* Command Specific Dword 3 */

    uint64_t mptr;     /* Metadata Pointer */
    
    /* DPTR : Data Pointer -- 128 bits */
    union {
        struct prps {
            uint64_t prp1;
            uint64_t prp2;
        } prp_t;
        uint64_t sgl[2];
    } dptr;

    uint32_t cdw10;
    uint32_t cdw11;
    uint32_t cdw12;
    uint32_t cdw13;
    uint32_t cdw14;
    uint32_t cdw15;
}  Nvme_sqe_t;

/* assert the size of Nvme_sqe_t at compile tiume to guarantee data alignement */
_Static_assert(sizeof(Nvme_sqe_t) == SQ_ENTRY_SIZE);

typedef struct nvme_cqe_t {
    uint32_t dw0;    /* Command Specific */
    uint32_t dw1;    /* Command Specific */
    uint16_t sqhd;   /* SQ Head Pointer */
    uint16_t sqid;   /* SQ Identifier */
    uint16_t cid;    /* Command Identifier */
    
    union {
        struct {
            uint16_t p  : 1;  /* Phase Tag (Bit 0) */
            uint16_t sf : 15; /* Status Field (Bits 1-15) */
        };
        uint16_t status_raw;  /* Pour l'accès direct */
    };
} __attribute__((packed)) Nvme_cqe_t;

/* assert the size of Nvme_cqe_t at compile tiume to guarantee data alignement */
_Static_assert(sizeof(Nvme_cqe_t) == CQ_ENTRY_SIZE);

/* Transport API to sendout commands to the NVMe controller */
int8_t nvme_send_command(volatile void *pci_bar, Nvme_sqe_t *sqe, Nvmeq_context_t *ctx, uint16_t qid); 

#endif /* NVME_TRANSPORT_H */