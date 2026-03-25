#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "nvme_spec.h"
#include "nvme_transport.h"

#include "nvme_sqe.h"

Nvme_sqe_t * nvme_create_set_features_sqe(uint8_t fid, uint32_t dword11) 
{
    static Nvme_sqe_t sqe;
    memset(&sqe, 0, sizeof(Nvme_sqe_t));

    sqe.opcode = NVME_ADM_OP_SET_FEATURES;
    sqe.cdw10 = fid;   
    sqe.cdw11 = dword11;

    return &sqe;
}

Nvme_sqe_t * nvme_create_iocompcreate_sqe(uint16_t qid, uint16_t size, uint64_t cq_phys)
{
    static Nvme_sqe_t sqe;
    memset(&sqe, 0, sizeof(Nvme_sqe_t));

    sqe.opcode = NVME_ADM_OP_CREATE_CQ; 
    sqe.dptr.prp_t.prp1 = cq_phys;
    
    sqe.cdw10 = ((uint32_t)(size - 1) << 16) | qid;
    sqe.cdw11 = NVME_QUEUE_PHYS_CONTIGUOUS; 

    return &sqe;
}

Nvme_sqe_t * nvme_create_iosubcreate_sqe(uint16_t qid, uint16_t size, uint64_t sq_phys, uint16_t cqid)
{
    static Nvme_sqe_t sqe;
    memset(&sqe, 0, sizeof(Nvme_sqe_t));

    sqe.opcode = NVME_ADM_OP_CREATE_SQ;
    sqe.dptr.prp_t.prp1 = sq_phys;

    /* CDW10: Queue Size (bits 16:31, 0-based) | QID (bits 0:15) */
    sqe.cdw10 = ((uint32_t)(size - 1) << 16) | qid;
    
    /* CDW11: CQID (bits 16:31) | PC bit (bit 0) */
    sqe.cdw11 = ((uint32_t)cqid << 16) | NVME_QUEUE_PHYS_CONTIGUOUS;

    return &sqe;
}

/*
 * nvme_create_read_io_sqe: Formats an I/O Submission Queue Entry for a 
 * Read operation. Sets the appropriate Opcode, LBA, and PRP fields.
 */
Nvme_sqe_t * nvme_create_read_io_sqe(uint64_t slba, uint16_t nlb, uint64_t prp1, uint64_t prp2) 
{
    (void) slba;
    (void) nlb;
    (void) prp1;
    (void) prp2;

    Nvme_sqe_t * sqe = calloc(1, sizeof(Nvme_sqe_t));
    if (sqe == NULL) {
        warn("Failed to create SQE request");
        return NULL;
    }
    return sqe;
}

/*
 * nvme_create_write_io_sqe: Formats an I/O Submission Queue Entry for a 
 * Write operation. Sets the appropriate Opcode, LBA, and PRP fields.
 */
Nvme_sqe_t * nvme_create_write_io_sqe(uint64_t slba, uint16_t nlb, uint64_t prp1, uint64_t prp2)
{
    (void) slba;
    (void) nlb;
    (void) prp1;
    (void) prp2;
    
    Nvme_sqe_t * sqe = calloc(1, sizeof(Nvme_sqe_t));
    if (sqe == NULL) {
        warn("Failed to create SQE request");
        return NULL;
    }
    return sqe;
}