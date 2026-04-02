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
#include "riscv_time.h"

#include "nvme_sqe.h"
#include "nvme_transport.h"
#include "nvme_queue_ctx.h"
#include "nvme_core.h"

#include "scheduler_ctx.h"
#include "IO_transport_ctx.h"
#include "priority_queues_ctx.h"
#include "workers.h"

/* IO_send: Format a Write/Read operation and send it to the NVMe controller

   Note: If the controller is full, IO_send return EXIT_FAILURE and it's up to
   the scheduler to handle it (reschedule, check for deadline and evict..)
*/
static int8_t IO_send(Async_transport_ctx *transport_ctx, Nvmeq_context_t *IOctx, uint16_t new_cid)
{
    /* create request */
    IO_metadata_t *meta = &transport_ctx->TaskTable[new_cid];

    /* Ensure we see all data written by the Dispatcher */
    atomic_thread_fence(memory_order_acquire);

    Nvme_sqe_t ioreq = nvme_create_io_sqe(
        atomic_load_explicit(&meta->opc,  memory_order_relaxed),
        atomic_load_explicit(&meta->slba, memory_order_relaxed),
        atomic_load_explicit(&meta->nlb,  memory_order_relaxed),
        atomic_load_explicit(&meta->prp1, memory_order_relaxed),
        atomic_load_explicit(&meta->prp2, memory_order_relaxed)
    );

    if (ioreq.opcode == NVME_IO_INVALID) {
        return -1;
    }

    ioreq.cid = new_cid;
    ioreq.nsid = atomic_load_explicit(&meta->nsid, memory_order_relaxed);

    /* Write to the ring buffer (DMA memory) */
    Nvme_sqe_t * target_slot = &((Nvme_sqe_t *) IOctx->sq_virt_addr)[IOctx->sq_tail];
    *target_slot = ioreq;


    if(transport_ctx->update_requests(transport_ctx, STATE_FREE, STATE_PENDING, STATUS_PENDING, new_cid) == EXIT_FAILURE) {
        return -1;
    }

    /* Ensure we see all data written by the Dispatcher */
    atomic_thread_fence(memory_order_acquire);

    /* Increment local tail index */
    IOctx->sq_tail = (IOctx->sq_tail + 1) % IOctx->sq_depth;
    /* Direct Register Write: Trigger the hardware doorbell */
    *((volatile uint32_t*) (uintptr_t) IOctx->sq_tdbl) = (uint32_t) IOctx->sq_tail;
    return 0;

}

static int8_t IO_receive(Async_transport_ctx *tctx, Nvmeq_context_t *IOctx)
{
    volatile Nvme_cqe_t *cq = (volatile Nvme_cqe_t *)IOctx->cq_virt_addr;
    
    while (1) {
        volatile Nvme_cqe_t *entry = &cq[IOctx->cq_head];
        
        /* Check Phase Tag for data freshness */
        if (entry->dw3.p != IOctx->expected_phase) {
            break;
        }

        /* Acquire barrier */
        MEM_FENCE(r, rw);

        uint16_t cid = entry->cid;
        uint8_t final_status = (entry->dw3.sf != 0) ? STATUS_ERR_NVME : STATUS_SUCCESS;
        
        if (tctx->update_requests(tctx, STATE_PENDING, STATE_FREE, final_status, cid) == EXIT_FAILURE) {
            return -1;
        }


        /* Update local SQ head from SQHD field */
        IOctx->sq_head = entry->sqhd;

        /* Advance CQ head and flip phase on wrap-around */
        IOctx->cq_head++;
        if (IOctx->cq_head == IOctx->cq_depth) {
            IOctx->cq_head = 0;
            IOctx->expected_phase ^= 1;
        }

        /* Update CQ Doorbell */
        *((volatile uint32_t*) (uintptr_t) IOctx->cq_hdbl) = (uint32_t) IOctx->cq_head;
    }

    return 0;
}


void * worker(void* arg)
{
    worker_arg_t *args = (worker_arg_t *) arg;
    Scheduler_ctx *self = args->self;
    uint8_t queue_ID = args->queue_ID;
    Nvmeq_context_t *io_ctx = &self->pqueues[queue_ID].io_ctx;

    if (!io_ctx || !self) {
        return NULL;
    }

    printf("Worker %d : Up and running.\n", queue_ID);

    while (self->running) {

        if (IO_receive(&self->tctx, io_ctx) == -1) {
            self->running = 0;
        }

        /* 
            Check if the submission queue is full, if it's the case, just
            call the IO_receive reaper once again
        */
        uint16_t next_tail = (io_ctx->sq_tail + 1) % io_ctx->sq_depth;
        if (next_tail == io_ctx->sq_head) {
            continue;
        }

        TObj task = self->pqueues[queue_ID].pop_Tobj(&self->pqueues[queue_ID]);
        /* Queue is empty so busy wait */
        if (task.cid == 0xFFFFFFFF) {
            atomic_store_explicit(&self->pqueues[queue_ID].service_time, 0, memory_order_relaxed);
            continue;
        }

        /* update the service time of the appropriate queue */
        atomic_fetch_sub(&self->pqueues[queue_ID].service_time, task.expected_duration);

        /* Check for task deadline */
        uint64_t current_time = get_riscv_tick();
        if (current_time > task.absolute_deadline) {
            /* Update the request */
            if(self->tctx.update_requests(&self->tctx, STATE_FREE, STATE_DONE, STATUS_DEADLINE_PASSED, task.cid) == EXIT_FAILURE) {
                self->running = 0;
                continue;
            }
        } else {
            /* Send the IO */
            if (IO_send(&self->tctx, io_ctx, task.cid)) {
                    L_ERR("Failed to create task", "nvme_create_io_sqe() error");
                    if (self->tctx.update_requests(&self->tctx, STATE_FREE, STATE_DONE, STATUS_ERR_NVME, task.cid) == EXIT_FAILURE) {
                        self->running = 0;
                    }
            }
        }
    }
    printf("Worker %d : Exiting.\n", queue_ID);
    return NULL;
}

void * reap_worker(void * arg)
{
    reaper_arg_t *args = (reaper_arg_t *) arg;
    Scheduler_ctx *self = args->self;
    rnd_bench_ctx_t* bench = args->bench;

    if (bench == NULL) {
        L_ERR("Reaper", "Benchmark context is NULL. Reaper exit.");
        return NULL;
    }

    while (self->running) {
        for (uint64_t i = 0; i < MAX_REQ_CAP; i++) {
            /* Atomic Take: Try to move state from DONE to FREE in one op */
            if (atomic_exchange_explicit(&self->tctx.TaskTable[i].state, 
                                       STATE_FREE, 
                                       memory_order_acq_rel) == STATE_DONE) {
                
                bench->requests_completed = bench->requests_completed + 1;
                /* Push CID back to free pool only if we won the exchange */
                self->tctx.push_cid(&self->tctx, i);
            }
        }
        __asm__ volatile ("pause" ::: "memory");
    }

    L_INFO("Garbage collection finished. reaper exiting.");
    return NULL;
}