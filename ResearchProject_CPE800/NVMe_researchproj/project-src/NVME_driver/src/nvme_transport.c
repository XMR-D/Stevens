#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "log.h"
#include "lockfree.h"
#include "nvme_transport.h"

/* * NVME_MAX_POLL_ITERATIONS : 
 * Based on ~5 cycles per loop (pause + inc + cmp + jmp).
 * For 500ms at 2GHz, we need ~200,000,000 iterations.
 */
#define MAX_POLL_FREQ 200000000ULL

/* nvme_trigger_doorbell: Triggers the NVMe controller doorbell register.
 *  is_sq : 1 for Submission Queue, 0 for Completion Queue
 *  qid   : 0 for Admin, 1+ for I/O Queues
 *  val   : New Tail (for SQ) or New Head (for CQ)
 */
static inline void nvme_trigger_doorbell(volatile void * pci_bar, uint8_t is_sq, uint16_t qid, uint32_t val)
{
    /* Calculation based on NVMe Spec: 1000h + (2 * QID * (4 << CAP.DSTRD)) */
    /* Assuming standard DSTRD = 0 (4 bytes stride) */
    uint32_t offset = 0x1000 + (qid * 8);

    if (!is_sq) {
        offset += 4; // CQ doorbell is 4 bytes after SQ
    }

    *(volatile uint32_t *)((uint8_t *)pci_bar + offset) = val;
}

/**
 * nvme_poll_completion: Waits for a completion entry by monitoring the Phase Tag.
 * Checks the Status Field for hardware or command errors.
 */
static int8_t nvme_poll_completion(Nvmeq_context_t *ctx, uint16_t qid, uint16_t cid)
{
    uint64_t timeout = 0;
    /* Use volatile to ensure the CPU re-reads the completion entry from RAM */
    volatile Nvme_cqe_t *current_cqe = (Nvme_cqe_t *)ctx->cq_virt_addr + ctx->cq_head;

    /* Wait for the controller to invert the Phase Tag (p) */
    while (current_cqe->p != ctx->expected_phase) {
        __builtin_riscv_pause();
        
        if (++timeout > MAX_POLL_FREQ) {
            L_ERR("Failed to submit command: ", "NVMe Timeout on QID and CID: ");
            printf("qid: %d | cid: %d\n", qid, cid);
            return EXIT_FAILURE;
        }
    }

    /* Ensure subsequent reads see the updated CQE data */
    MEM_FENCE(r, r);

    /* Validate that the Status Field (sf) is zero (Success) */
    if (current_cqe->sf != 0) {
        return EXIT_FAILURE; 
    }

    return EXIT_SUCCESS;
}

/**
 * nvme_send_command: Submits an SQE to the specified queue, updates the 
 * submission doorbell, and synchronously polls for the completion result.
 */
int8_t nvme_send_command(volatile void *pci_bar, Nvme_sqe_t *sqe, Nvmeq_context_t *ctx, uint16_t qid) 
{
    /* Copy command to the ring buffer and ensure visibility before doorbell */
    Nvme_sqe_t *target_slot = (Nvme_sqe_t *)ctx->sq_virt_addr + ctx->sq_tail;
    *target_slot = *sqe;
    MEM_FENCE(w, w);

    /* Update the submission tail index and notify the controller */
    ctx->sq_tail = (ctx->sq_tail + 1) % ctx->sq_depth;
    nvme_trigger_doorbell(pci_bar, 1, qid, ctx->sq_tail);

    /* Synchronous wait for the completion entry */
    if (nvme_poll_completion(ctx, qid, sqe->cid) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }

    /* Move completion head and toggle phase bit if the queue wrapped around */
    ctx->cq_head = (ctx->cq_head + 1) % ctx->cq_depth;    
    if (ctx->cq_head == 0) {
        ctx->expected_phase = !ctx->expected_phase;
    }
    
    /* Acknowledge processed completion entry to the controller */
    nvme_trigger_doorbell(pci_bar, 0, qid, ctx->cq_head);

    return EXIT_SUCCESS;
}



/*
    
    TODO: CHANGE THE LOGIC LATER TO MORPH COMMAND SUBMISSION AND RECEPTION TO BE
    ASYNCHRONOUS TO AVOID CONTENTION



 * nvme_read_cqe: Fetches the next entry from the Completion Queue 
 * at the physical address specified in the nvmeq_ctx.
 * Checks the Phase Tag (P) to determine if the entry is valid/new.

Nvme_cqe_t * nvme_read_cqe(volatile void * pci_bar, Nvmeq_context_t * nvmeq_ctx)
{
    (void) nvmeq_ctx;
    return NULL;
}
*/