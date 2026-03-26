#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "macros.h"

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


/* ASYNCHRONOUS COMMANDS */

/*
 * nvme_create_write_io_sqe: Formats an I/O Submission Queue Entry for a 
 * IO operation.
 */
Nvme_sqe_t nvme_create_io_sqe(uint8_t opcode, uint64_t slba, uint16_t nlb, uint64_t prp1, uint64_t prp2)
{

    Nvme_sqe_t sqe = {0};

    /* Anti-underflow barrier*/
    if (nlb <= 0) {
        L_WARN("Invalid or suprious IO command detected", "Underflow attempted using nlb");
        sqe.opcode = NVME_IO_INVALID;
        return sqe;
    }

    // CDW0 & NSID
    sqe.opcode = opcode;
    sqe.nsid = 1; // Default Namespace
    
    // DPTR (Data Pointer)
    sqe.dptr.prp_t.prp1 = prp1;
    sqe.dptr.prp_t.prp2 = prp2;

    /* * Manual mapping on CDWs (Command Specific)
     * CDW10 : SLBA[31:00]
     * CDW11 : SLBA[63:32]
     * CDW12 : NLB (bits 15:00)
     */
    sqe.cdw10 = (uint32_t)(slba & 0xFFFFFFFF);
    sqe.cdw11 = (uint32_t)(slba >> 32);
    sqe.cdw12 = (uint16_t)(nlb - 1);

    return sqe;
}
