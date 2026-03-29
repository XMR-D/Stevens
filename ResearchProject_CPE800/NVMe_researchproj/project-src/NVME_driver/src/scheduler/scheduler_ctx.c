#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/mman.h>

#include "macros.h"
#include "benchmark.h"

#include "nvme_sqe.h"
#include "nvme_transport.h"
#include "nvme_queue_ctx.h"
#include "nvme_core.h"

#include "scheduler_ctx.h"
#include "IO_transport_ctx.h"
#include "priority_queues_ctx.h"
#include "workers.h"



/* Scheduler context destructor */
void _destroy(Scheduler_ctx * self)
{
    if (self) {

        self->running = 0;

        for (int i = 0; i < NB_PRIO_QUEUE; i++) {
            if (self->worker_threads[i] != 0) {
                if (pthread_join(self->worker_threads[i], NULL) != 0) {
                    L_ERR("Thread Join", "Failed to join worker :");
                    printf("worker_id : %d\n", i);
                }
            }
        }
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

void _submit_task(Scheduler_ctx *self, uint16_t cid, uint64_t absolute_deadline, uint64_t timestamp_start, uint64_t queue_ID,
    uint8_t opc, uint32_t nsid, uint64_t slba, uint16_t nlb, uint64_t prp1, uint64_t prp2)
{

    atomic_store_explicit(&self->tctx.TaskTable[cid].slba, slba, memory_order_relaxed);
    atomic_store_explicit(&self->tctx.TaskTable[cid].prp1, prp1, memory_order_relaxed);
    atomic_store_explicit(&self->tctx.TaskTable[cid].prp2, prp2, memory_order_relaxed);
    atomic_store_explicit(&self->tctx.TaskTable[cid].nsid, nsid, memory_order_relaxed);
    atomic_store_explicit(&self->tctx.TaskTable[cid].nlb, nlb, memory_order_relaxed);
    atomic_store_explicit(&self->tctx.TaskTable[cid].opc, opc, memory_order_relaxed);
    atomic_store_explicit(&self->tctx.TaskTable[cid].absolute_deadline, absolute_deadline, memory_order_relaxed);
    atomic_store_explicit(&self->tctx.TaskTable[cid].timestamp_start, timestamp_start, memory_order_relaxed);
    atomic_store_explicit(&self->tctx.TaskTable[cid].queue_ID, queue_ID, memory_order_relaxed);

    /* submit the task to the appropriate queue */
    self->pqueues[queue_ID].push_Tobj(&self->pqueues[queue_ID], cid, absolute_deadline, timestamp_start);

}

void _dispatch_loop(Scheduler_ctx *self, rnd_bench_ctx_t* bench)
{
    /* Parse requests from benchmark */
    L_INFO("Dispatcher loop reached");
    while (self->running) {
        bench = bench + 1;
        continue;
    }
    L_INFO("Dispatch finished for the actual benchmark, destroying scheduler, so long...");
}

/* this function create the worker thread using a wrapper for IO_send polling and 
   associate them with a priority queue it create the completion read thread using IO_read

   once every thread launched, poll for upcomming workloads from the benchmark layer
*/
void _start_scheduler(Scheduler_ctx *self, rnd_bench_ctx_t* bench)
{
    /* first init the workers */
    for (uint8_t i = 0; i < NB_PRIO_QUEUE; i++) {
        self->worker_ids[i] = i;

        self->thread_args[i].self = self;
        self->thread_args[i].queue_ID = i;
        
        if (pthread_create(&self->worker_threads[i], NULL, worker, &self->thread_args[i]) != 0) {
            L_ERR("Thread Init", "Failed to spawn sender worker");
            _destroy(self);
            return;
        }
    }

    L_SUCC("Scheduler: Worker threads created, starting dispatcher");
    self->running++;
    _dispatch_loop(self, bench);

    return;

}


/* 
    SCHEDULER CONSTRUCTOR
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

    obj->start_scheduler = _start_scheduler;
    obj->submit_task = _submit_task;
    obj->log_scheduler = _log_scheduler;
    obj->destroy = _destroy;

    return obj;

}
