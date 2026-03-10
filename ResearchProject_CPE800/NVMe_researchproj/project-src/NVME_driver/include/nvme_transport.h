#ifndef NVME_TRANSPORT_H
#define NVME_TRANSPORT_H

#include <stdint.h>

#include "nvme_q.h"
/* NVME QUEUES ENTRIES FORMAT */

#define NVME_SQE_SIZE 64
#define NVME_CQE_SIZE 16


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
_Static_assert(sizeof(Nvme_sqe_t) == NVME_SQE_SIZE);

/*
* Nvme_cqe_t : Definition of the Completion Queue Entry (SQE)
* Format : 16 bytes total
* (NVM Express® Base Specification, Revision 2.2, p95-96)
*/
typedef struct nvme_cqe_t {
    /* Command Specific */
    uint32_t dw0;
    uint32_t dw1;

    /* DWORD 2 -- SQ Identifier and Head Pointer */
    uint16_t sqhd;
    uint16_t sqid;

    /* DWORD 3 -- Status field, Phase Tag, Command identifier */
    uint16_t cid;
    uint16_t p : 1;
    uint16_t sf : 15;

} Nvme_cqe_t ;

/* assert the size of Nvme_cqe_t at compile tiume to guarantee data alignement */
_Static_assert(sizeof(Nvme_cqe_t) == NVME_CQE_SIZE);

Nvme_sqe_t * nvme_create_sqe(Nvme_sqe_t * sqe);
int8_t nvme_send_sqe(Nvme_sqe_t * sqe, Nvmeq_context_t * nvmeq_ctx);

Nvme_cqe_t * nvme_read_cqe(Nvmeq_context_t * nvmeq_ctx);
int8_t nvme_parse_cqe(Nvme_cqe_t * cqe);

static inline void nvme_trigger_doorbell(volatile void * pci_bar);

#endif /* NVME_TRANSPORT_H */