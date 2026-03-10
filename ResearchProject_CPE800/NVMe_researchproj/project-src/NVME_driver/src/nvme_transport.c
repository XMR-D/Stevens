#include <err.h>
#include <errno.h>

#include "nvme_transport.h"

/* * nvme_send_sqe: Copies the prepared SQE into the Submission Queue memory 
 * buffer located at the physical address stored in nvmeq_ctx.
 * Updates the Doorbell register to notify the controller of a new command.
 */
int8_t nvme_send_sqe(Nvme_seq_t * sqe, Nvmeq_context_t * nvmeq_ctx) 
{
    return EXIT_SUCCESS;
}

/* * nvme_read_cqe: Fetches the next entry from the Completion Queue 
 * at the physical address specified in the nvmeq_ctx.
 * Checks the Phase Tag (P) to determine if the entry is valid/new.
 */
Nvme_cqe_t * nvme_read_cqe(Nvmeq_context_t * nvmeq_ctx)
{
    return NULL;
}

/* * nvme_parse_cqe: Analyzes the Status Field (SF) of the completed entry.
 * Validates the status code to check for errors or successful completion,
 * and correlates the CID with the original submission.
 */
int8_t nvme_parse_cqe(Nvme_cqe_t * cqe)
{
    return EXIT_SUCCESS;
}

/* * nvme_trigger_doorbell: Trigger the NVME controller doorbell to signal
 *   that an operation needs to be performed by the controller.
 */
static inline void nvme_trigger_doorbell(volatile void * pci_bar)
{
    return;
}
