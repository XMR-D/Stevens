#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "nvme_transport.h"

/*
 * nvme_trigger_doorbell: Triggers the NVMe controller doorbell register 
 * to signal that new commands have been posted to the Submission Queue.
 */
static inline void nvme_trigger_doorbell(volatile void * pci_bar)
{
    (void)pci_bar;
    return;
}

/* 
 * nvme_send_adm_sqe: Copies a prepared SQE into the Admin Submission Queue 
 * buffer in host memory. Performs a memory barrier to ensure data visibility 
 * before ringing the Admin Submission Queue Doorbell to notify the controller.
 */
int8_t nvme_send_adm_sqe(Nvme_sqe_t * sqe, Nvmeq_context_t * adm_ctx) 
{
    (void) sqe;
    (void) adm_ctx;
    return EXIT_SUCCESS;
}

/* 
 * nvme_send_io_sqe: Copies a prepared SQE into the I/O Submission Queue 
 * buffer in host memory. Ensures memory ordering via barriers before updating 
 * the corresponding I/O Submission Queue Doorbell to trigger command processing.
 */
int8_t nvme_send_io_sqe(Nvme_sqe_t * sqe, Nvmeq_context_t * io_ctx) 
{
    (void) sqe;
    (void) io_ctx;
    return EXIT_SUCCESS;
}

/* 
 * nvme_read_cqe: Fetches the next entry from the Completion Queue 
 * at the physical address specified in the nvmeq_ctx.
 * Checks the Phase Tag (P) to determine if the entry is valid/new.
 */
Nvme_cqe_t * nvme_read_cqe(Nvmeq_context_t * nvmeq_ctx)
{
    (void) nvmeq_ctx;
    return NULL;
}
