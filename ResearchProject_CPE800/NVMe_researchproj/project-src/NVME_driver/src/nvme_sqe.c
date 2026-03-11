#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

#include "nvme_spec.h"
#include "nvme_transport.h"

#include "nvme_sqe.h"

/*
 * nvme_create_iocreate_adm_sqe: Formats a Submission Queue Entry (SQE) 
 * for the Identify command used during Admin Queue initialization.
 */
Nvme_sqe_t * nvme_create_iosubcreate_sqe(uint16_t qid, uint16_t size, uint64_t sq_phys)
{
    (void) qid;
    (void) size;
    (void) sq_phys;
    return NULL;
}


/*
 * nvme_create_identify_adm_sqe: Formats a Submission Queue Entry (SQE) 
 * for the Identify command used during Admin Queue initialization.
 */
Nvme_sqe_t * nvme_create_iocompcreate_sqe(uint16_t qid, uint16_t size, uint64_t cq_phys)
{
    (void) qid;
    (void) size;
    (void) cq_phys;
    return NULL;
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
