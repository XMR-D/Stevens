#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>


#include <sys/mman.h>

#include "macros.h"

#include "nvme_sqe.h"
#include "nvme_transport.h"
#include "nvme_queue_ctx.h"
#include "nvme_core.h"

#include "scheduler_ctx.h"
#include "IO_transport_ctx.h"
#include "priority_queues_ctx.h"

/* Transport SPSC context destructor */
void _destroy(Scheduler_ctx * self)
{
    if (self) {
        free(self);
    }
}

void _log_scheduler(Scheduler_ctx *self) 
{
    if (!self) {
        L_ERR("Scheduler Log", "Context is NULL");
        return;
    }

    printf("\n--- [ Scheduler Transport Context Overview ] ---\n");
    printf("Number of active Prio Queues: %d\n", NB_PRIO_QUEUE);
    printf("------------------------------------------------\n");

    for (uint8_t i = 0; i < NB_PRIO_QUEUE; i++) {
        PQueueObj *q = &self->pqueues[i];
        
        printf("Queue [%d] (QID: %d):\n", i, i + 1);
        printf("  -> Virtual SQ Base: 0x%016lx\n", q->io_ctx.sq_virt_addr);
        printf("  -> Virtual CQ Base: 0x%016lx\n", q->io_ctx.cq_virt_addr);
        printf("  -> Doorbell SQ Tail: 0x%016x\n", q->io_ctx.sq_tdbl);
        printf("  -> Doorbell CQ Head: 0x%016x\n", q->io_ctx.cq_hdbl);
        printf("  -> Current Indices: SQ_T=%u | CQ_H=%u\n", q->io_ctx.sq_tail, q->io_ctx.cq_head);
        printf("  -> Expected Phase:  %d\n", q->io_ctx.expected_phase);
        printf("------------------------------------------------\n");
    }
    
    L_SUCC("Scheduler status logged successfully");
}


/* IO_receive: Poll to read incoming requests and update the dictionary to signal scheduler
   a task was completed.
*/
void IO_receive(Async_transport_ctx *transport_ctx, Nvmeq_context_t *IOctx)
{
    Nvme_cqe_t *cqe = &((Nvme_cqe_t *)IOctx->cq_virt_addr)[IOctx->cq_head];
    uint8_t processed = 0;

    /* Poll while the Phase Tag (P) matches our expected phase */
    while ((cqe->dw3.p & 0x1) == IOctx->expected_phase) {
        processed = 1;
        uint16_t cid = cqe->cid;

        uint16_t status = (cqe->dw3.sf >> 1);

        transport_ctx->update_requests(transport_ctx, cid, 2);

        if (status != 0) {
            L_ERR("NVMe IO Error", "Command Status indicating failed operation | command CID = ");
            printf("cid = %d", cid);
        }
        
        IOctx->cq_head++;
        if (IOctx->cq_head >= IOctx->cq_depth) {
            IOctx->cq_head = 0;
            /* Flip phase bit on wrap-around */
            IOctx->expected_phase ^= 1; 
        }

        cqe = &((Nvme_cqe_t *)IOctx->cq_virt_addr)[IOctx->cq_head];
    }

    /* If we processed completions, update the hardware Head Doorbell */
    if (processed) {
        *((volatile uint32_t*) (uintptr_t) IOctx->cq_hdbl) = (uint32_t) IOctx->cq_head;
    }
}


/* IO_send: Format a Write/Read operation and send it to the NVMe controller

   Note: If the controller is full, IO_send return EXIT_FAILURE and it's up to
   the scheduler to handle it (reschedule, check for deadline and evict..)
*/
int32_t IO_send(Async_transport_ctx *transport_ctx, Nvmeq_context_t *IOctx,  uint8_t opc, uint32_t nsid, uint64_t slba, uint16_t nlb, uint64_t prp1, uint64_t prp2)
{
    
    uint16_t new_cid = transport_ctx->pop_cid(transport_ctx);
    
    if (new_cid != 0xFFFF) {

        /* create request */
        Nvme_sqe_t ioreq = nvme_create_io_sqe(opc, slba, nlb, prp1, prp2);
        if (ioreq.opcode == NVME_IO_INVALID) {
            return -1;
        }

        ioreq.cid = new_cid;
        ioreq.nsid = nsid;

        /* Write to the ring buffer (DMA memory) */
        Nvme_sqe_t * target_slot = &((Nvme_sqe_t *) IOctx->sq_virt_addr)[IOctx->sq_tail];
        *target_slot = ioreq;

        /* Ensure the command is written to RAM before triggering the doorbell */
        MEM_FENCE(w, w);

        /* Increment local tail index */
        IOctx->sq_tail = (IOctx->sq_tail + 1) % IOctx->sq_depth;
        
        /* Direct Register Write: Trigger the hardware doorbell */
        *((volatile uint32_t*) (uintptr_t) IOctx->sq_tdbl) = (uint32_t) IOctx->sq_tail;

        transport_ctx->update_requests(transport_ctx, new_cid, 1);
        return (int32_t) new_cid;
    }
    return -1;
}

/* this function create the worker thread using a wrapper for IO_send polling and 
   associate them with a priority queue it create the completion read thread using IO_read

   once every thread launched, poll for upcomming workloads from the benchmark layer
*/
void _start_scheduler(void)
{
    //TODO
}

/* 
   Transport SPSC context constructor.
   
   Note: PLEASE USE A FENCE AFTER CALLING THE FUNCTION TO ENSURE THE STRUCTURE IS CORRECTLY 
   INSTANCIATED AND THREADS DO NOT START OPERATIONS ON A ILL CONTEXT
*/
Scheduler_ctx * create_scheduler_context(volatile void * bar, Nvmeq_context_t * admin_ctx)
{
    Scheduler_ctx * obj = calloc(1, sizeof(Scheduler_ctx)); 
    if (!obj) {
        L_ERR("Failed to create Scheduler transport context (see scheduler_ctx.c l.28)", "calloc failed");
        return NULL;
    }

    int64_t pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap_fd < 0) {
        _destroy(obj);
        return NULL;
    }

    L_INFO("Attempting to initialize Asynchronous context");
    tctx_class_init(&obj->tctx);

    /* STEP 1 : Request  NB_PRIO_QUEUE SQ/CQ queue pairs */
    uint32_t q_count = ((NB_PRIO_QUEUE - 1) << 16) | (NB_PRIO_QUEUE - 1);
    L_INFO("Sending Set feature request");
    if (admin_send(bar, nvme_create_set_features_sqe(0x07, q_count), admin_ctx, 0)) 
        return NULL;
    L_SUCC("Success");

    /* for each context init an I/O nvme queue pair and send a create command */
    for (uint8_t i = 0; i < NB_PRIO_QUEUE; i++) {
        L_INFO("Attempting to initialize new prio queue : ");
        if (pqueue_class_init(bar, admin_ctx, &(obj->pqueues[i]), pagemap_fd, i+1) == EXIT_FAILURE) {
            _destroy(obj);
            return NULL;
        }
    }

    L_SUCC("Scheduler Context created successfully");

    obj->destroy = _destroy;
    obj->log_scheduler = _log_scheduler;
    obj->start_scheduler = _start_scheduler;

    return obj;

}
