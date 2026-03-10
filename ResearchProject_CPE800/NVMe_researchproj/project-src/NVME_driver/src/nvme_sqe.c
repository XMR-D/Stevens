#include <err.h>
#include <errno.h>

#include "nvme_transport.h"

#include "nvme_sqe.h"


/* * nvme_create_read_sqe: Formats a Submission Queue Entry based on a template.
 * Sets up necessary command fields (Opcode, CID, etc.) in the provided
 * SQE structure before it is posted to the Submission Queue.
 */
Nvme_sqe_t * nvme_create_read_sqe(Nvme_seq_t * sqe) 
{
    return NULL;
}

/* * nvme_create_write_sqe: Formats a Submission Queue Entry based on a template.
 * Sets up necessary command fields (Opcode, CID, etc.) in the provided
 * SQE structure before it is posted to the Submission Queue.
 */
Nvme_sqe_t * nvme_create_write_sqe(Nvme_seq_t * sqe)
{
    return NULL;
}
