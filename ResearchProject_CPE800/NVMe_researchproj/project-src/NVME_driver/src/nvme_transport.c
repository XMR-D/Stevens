#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include "macros.h"
#include "nvme_sqe.h"
#include "nvme_transport.h"
#include "IO_transport_ctx.h"


/*
    SYNCHRONOUS MODE FOR HANDLING COMMANDS TO ADMIN QUEUES
*/

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
inline void nvme_trigger_doorbell(volatile void * pci_bar, uint8_t is_sq, uint16_t qid, uint32_t val)
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
    while (current_cqe->dw3.p != ctx->expected_phase) {
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
    if (current_cqe->dw3.sf != 0) {
        return EXIT_FAILURE; 
    }

    return EXIT_SUCCESS;
}

/**
 * nvme_send_command: Submits an SQE to the specified queue, updates the 
 * submission doorbell, and synchronously polls for the completion result.
 */
int8_t admin_send(volatile void *pci_bar, Nvme_sqe_t *sqe, Nvmeq_context_t *ctx, uint16_t qid) 
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
    ASYNCHRONOUS MODE TO HANDLE IO REQUESTS TO THE IO_CONTEXT
*/

/* IO_transport_init: Initialize the Input/Output asynchronous transport layer
   In order to keep track of submited IO requests and completions status.
*/
Async_transport_ctx * IO_transport_ctx_init(void) 
{
    Async_transport_ctx * io_transport_ctx = create_asynch_transport_context();
    MEM_FENCE(rw, rw);

    return io_transport_ctx;
}

/*
static void log_nvme_sqe(Nvme_sqe_t *sqe) {
    if (!sqe) return;

    printf("\n--- [NVMe SQE DEBUG DUMP] ---\n");
    
    // Affichage logique (Champs individuels)
    printf("CDW0: OPC=0x%02X | CID=0x%04X | FUSE=0x%X | PSDT=0x%X\n", 
            sqe->opcode, sqe->cid, sqe->fuse, sqe->psdt);
    printf("NSID: 0x%08X\n", sqe->nsid);
    printf("MPTR: 0x%016lX\n", sqe->mptr);
    printf("PRP1: 0x%016lX\n", sqe->dptr.prp_t.prp1);
    printf("PRP2: 0x%016lX\n", sqe->dptr.prp_t.prp2);
    printf("CDWs: [10:0x%08X] [11:0x%08X] [12:0x%08X]\n", 
            sqe->cdw10, sqe->cdw11, sqe->cdw12);
    printf("      [13:0x%08X] [14:0x%08X] [15:0x%08X]\n", 
            sqe->cdw13, sqe->cdw14, sqe->cdw15);

    // Affichage Raw (Hex dump pour vérifier le packing de 64 octets)
    printf("\nRaw Binary (64 bytes):\n");
    const uint8_t *raw = (const uint8_t *)sqe;
    for (int i = 0; i < 64; i++) {
        printf("%02X ", raw[i]);
        if ((i + 1) % 16 == 0) printf("\n");
        else if ((i + 1) % 4 == 0) printf("| ");
    }
    printf("-----------------------------\n");
}
*/

/* IO_send: Format a Write/Read operation and send it to the NVMe controller

   Note: If the controller is full, IO_send return EXIT_FAILURE and it's up to
   the scheduler to handle it (reschedule, check for deadline and evict..)
*/
int32_t IO_send(Nvmeq_context_t *IOctx, Async_transport_ctx *transport_ctx, uint8_t opc, uint32_t nsid, uint64_t slba, uint16_t nlb, uint64_t prp1, uint64_t prp2) {
    
    uint16_t new_cid = transport_ctx->pop_cid(transport_ctx);
    
    if (new_cid != 0xFFFF) {

        Nvme_sqe_t ioreq = nvme_create_io_sqe(opc, slba, nlb, prp1, prp2);
        if (ioreq.opcode == NVME_IO_INVALID) {
            return -1;
        }

        ioreq.cid = new_cid;
        ioreq.nsid = nsid;

        /* Copy command to the ring buffer and ensure visibility before doorbell */
        Nvme_sqe_t * target_slot = &((Nvme_sqe_t *) IOctx->sq_virt_addr)[IOctx->sq_tail];
        *target_slot = ioreq;
        MEM_FENCE(w, w);

        /* Update the submission tail index and notify the controller */
        IOctx->sq_tail = (IOctx->sq_tail + 1) % IOctx->sq_depth;
        transport_ctx->update_requests(transport_ctx, new_cid, 1);
        MEM_FENCE(w, w);

        return (int32_t) new_cid;
    }
    return -1;
}

/* IO_receive: Poll to read incoming requests and update the dictionary to signal scheduler
   a task was completed.
*/
void IO_receive(Nvmeq_context_t *IOctx, Async_transport_ctx *transport_ctx)
{
    Nvme_cqe_t *cqe = &((Nvme_cqe_t *)IOctx->cq_virt_addr)[IOctx->cq_head];

    while ((cqe->dw3.p & 0x1) == IOctx->expected_phase) {
        
        uint16_t cid = cqe->cid;
        uint16_t status = (cqe->dw3.sf >> 1);

        transport_ctx->update_requests(transport_ctx, cid, 2);

        if (status != 0) {
            L_ERR("NVMe IO Error", "Command Status indicating failed operation | command CID = ");
            printf("cid = %d", cid);
            continue;
        }

        printf("command with cid : %d : SUCCESS\n", cid);

        IOctx->cq_head++;
        if (IOctx->cq_head >= IOctx->cq_depth) {
            IOctx->cq_head = 0;
            IOctx->expected_phase ^= 1;
        }

        cqe = &((Nvme_cqe_t *)IOctx->cq_virt_addr)[IOctx->cq_head];
    }
}
