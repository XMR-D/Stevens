#define _GNU_SOURCE

#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <sched.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/mman.h>

#include "macros.h"
#include "benchmark.h"
#include "riscv_time.h"

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

        for (int i = 0; i < NB_PRIO_QUEUE; i++) {
            if (self->worker_threads[i] != 0) {
                if (pthread_join(self->worker_threads[i], NULL) != 0) {
                    L_ERR("Thread Join", "Failed to join worker :");
                    printf("worker_id : %d\n", i);
                }
            }
        }

        if (pthread_join(self->reap_thread, NULL) != 0) {
            L_ERR("Thread Join", "Failed to join reaper :");
        }
        free(self);
    }
}


static inline const char* statustostr(uint8_t status)
{
    switch(status) {
        case 0:
            return "SUCC";
        case 1:
            return "PEND";
        case 2:
            return "ERRN";
        case 3:
            return "DEAD";
        default:
            return "UNDF";
    }
}

static inline const char* statetostr(uint8_t state, uint64_t* nb_free, uint64_t* nb_pend, uint64_t* nb_done)
{
    switch (state) {
        case 0:
            *nb_free = *nb_free + 1;
            return "FREE";
        case 1:
            *nb_pend = *nb_pend + 1;
            return "PEND";
        case 2:
            *nb_done = *nb_done + 1;
            return "DONE";
        default:
            return "UDST";
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
        printf("  -> Doorbell SQ Tail: 0x%016lx\n", q->io_ctx.sq_tdbl);
        printf("  -> Doorbell CQ Head: 0x%016lx\n", q->io_ctx.cq_hdbl);
        printf("  -> Current Indices: SQ_T=%u | CQ_H=%u\n", q->io_ctx.sq_tail, q->io_ctx.cq_head);
        printf("  -> Expected Phase:  %d\n", q->io_ctx.expected_phase);
        printf("  -> Current service time: %ld\n", q->service_time);
        printf("------------------------------------------------\n");
    }

    printf("Task Table breakdown :\n");
    uint64_t nb_FREE = 0;
    uint64_t nb_PEND = 0;
    uint64_t nb_DONE = 0;

    for (uint64_t i = 0; i < MAX_REQ_CAP; i++) {
        const char * str = statetostr(self->tctx.TaskTable[i].state, &nb_FREE, &nb_PEND, &nb_DONE);
        if (i <= 20) {
            printf("| %05ld | %s | %s |", i, str, statustostr(self->tctx.TaskTable[i].status));
            if (i == 9) {
                printf("\n");
            }
        }
    }

    printf("\n");
    printf("Number of free tasks in table : %ld\n", nb_FREE);
    printf("Number of pending tasks in table : %ld\n", nb_PEND);
    printf("Number of done tasks in table : %ld\n", nb_DONE);
    printf("\n");

    printf("Available cids : %d\n", (self->tctx.tail - self->tctx.head));

    L_SUCC("Scheduler status logged successfully");
}

/* Create and submit a task to the appropriate queue */
static inline void submit_task(Scheduler_ctx *self, uint16_t cid, uint32_t queue_id, const bench_req_t *task)
{
    /* Use local pointer for readability and potential compiler optimization */
    volatile IO_metadata_t *entry = &self->tctx.TaskTable[cid];

    /* Direct extraction from the task structure */
    entry->slba = task->slba;
    entry->prp1 = task->prp1;
    entry->prp2 = task->prp2;
    entry->nsid = task->nsid;
    entry->nlb = task->nlb;
    entry->opc = task->opc;
    entry->absolute_deadline = task->absolute_deadline;
    entry->timestamp_start = task->timestamp_submit;
    entry->queue_ID = queue_id;

    atomic_thread_fence(memory_order_release);

    /* Submit to the priority queue's object manager */
    atomic_fetch_add(&self->pqueues[queue_id].service_time, task->expected_duration);

    self->pqueues[queue_id].push_Tobj(&self->pqueues[queue_id], cid, task->absolute_deadline, task->timestamp_submit);
}

static inline uint32_t queue_select(Scheduler_ctx *self, bench_req_t * req)
{
    for (uint32_t i = 0; i < NB_PRIO_QUEUE; i++) {
        uint64_t q_service_time = atomic_load_explicit(&self->pqueues[i].service_time, memory_order_relaxed);
        if (q_service_time + req->expected_duration <= req->latency_budget_ticks) {
            return i;
        }
    }
    return 0xFFFFFFFF;
}


void _dispatch_loop(Scheduler_ctx *self, rnd_bench_ctx_t* bench)
{
    bench_req_t generated_task;
    /* Parse requests from benchmark */
    L_INFO("Dispatcher loop reached");
    while (1) {
        /* Retreive the task from the benchmark */
        if (!get_next_bench_request(bench, &generated_task)) {
            self->dispatch_finished = 1;
            break;
        }

        /* Estimate the absolute deadline required for the operation */
        /* Read / Write time based on the task size */
        /* Decide based on the service time of each queue where to place task */

        uint32_t t_cid = self->tctx.pop_cid(&self->tctx);
        if (t_cid == 0xFFFFFFFF) {
            bench->requests_not_accepted++;
            bench->drop_reason_no_cid++;
            continue;
        }

        uint32_t queue_id = queue_id = queue_select(self, &generated_task);
        if (queue_id == 0xFFFFFFFF) {
            bench->requests_not_accepted++;
            bench->drop_reason_queue_full++;
            continue;
        }

        uint32_t head = atomic_load_explicit(&self->pqueues[queue_id].head, memory_order_acquire);
        uint32_t tail = atomic_load_explicit(&self->pqueues[queue_id].tail, memory_order_relaxed);

        /* check if the queue is full */
        if ((tail - head) < PQUEUE_CAP) {
            submit_task(self, t_cid, queue_id, &generated_task);
        } else {
            /* mark the cid free again and mark the request as not accepted */
            self->tctx.push_cid(&self->tctx, t_cid);
            bench->requests_not_accepted++;
        }
    }
    L_INFO("Dispatch finished for the actual benchmark, destroying scheduler, so long...");
}


static void pin_thread_to_core(pthread_t thread, int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    if (pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset) != 0) {
        printf("Affinity: Failed to pin thread to core %d", core_id);
    } else {
        printf("[INFO] Thread pinned to core %d\n", core_id);
    }
}

/* this function create the worker thread using a wrapper for IO_send polling and 
   associate them with a priority queue it create the completion read thread using IO_read

   once every thread launched, poll for upcomming workloads from the benchmark layer
*/
void _start_scheduler(Scheduler_ctx *self, rnd_bench_ctx_t* bench)
{

    /* core 0 is used by the main thread */
    int current_core = 1;

    /* first init the workers */
    for (uint8_t i = 0; i < NB_PRIO_QUEUE; i++) {
        self->worker_ids[i] = i;
        self->worker_states[i] = 1;
        self->thread_args[i].self = self;
        self->thread_args[i].queue_ID = i;
        self->thread_args[i].bench = bench;
        
        if (pthread_create(&self->worker_threads[i], NULL, worker, &self->thread_args[i]) != 0) {
            L_ERR("Thread Init", "Failed to spawn sender worker");
            _destroy(self);
            return;
        }
        pin_thread_to_core(self->worker_threads[i], current_core++);
    }
    L_SUCC("Scheduler: Request submission worker threads created");

    self->reap_arg.self = self;
    self->reap_arg.bench = bench;
    L_INFO("Creating reaper thread");
    if (pthread_create(&self->reap_thread, NULL, reap_worker, &self->reap_arg) != 0) {
        L_ERR("Thread Init", "Failed to spawn reaper worker");
        _destroy(self);
        return;
    }
    L_SUCC("Scheduler: reaper thread created, starting dispatcher");

    self->dispatch_finished = 0;
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

    /* Request  NB_PRIO_QUEUE SQ/CQ queue pairs */
    uint32_t q_count = ((NB_PRIO_QUEUE - 1) << 16) | (NB_PRIO_QUEUE - 1);
    L_INFO("Sending Set feature request");
    if (admin_send(bar, nvme_create_set_features_sqe(0x07, q_count), admin_ctx, 0)) 
        return NULL;
    L_SUCC("Success");

    /* for each context init an I/O nvme queue pair and send a create command */
    for (uint8_t i = 0; i < NB_PRIO_QUEUE; i++) {
        L_INFO("Attempting to initialize new prio queue");
        printf("[NEW] IO Queue %d BAR base: %p\n", i, bar);
        if (pqueue_class_init(bar, admin_ctx, &(obj->pqueues[i]), pagemap_fd, i+1) == EXIT_FAILURE) {
            _destroy(obj);
            return NULL;
        }
    }

    L_SUCC("Scheduler Context created successfully");

    obj->start_scheduler = _start_scheduler;
    obj->log_scheduler = _log_scheduler;
    obj->destroy = _destroy;

    return obj;

}
