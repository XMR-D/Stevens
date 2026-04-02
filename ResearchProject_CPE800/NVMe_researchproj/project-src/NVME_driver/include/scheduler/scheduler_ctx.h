#ifndef SCHEDULER_CTX_H
#define SCHEDULER_CTX_H

#include <pthread.h>

#include "benchmark.h"

#include "nvme_queue_ctx.h"

#include "IO_transport_ctx.h"
#include "priority_queues_ctx.h"


typedef struct scheduler_ctx Scheduler_ctx;

#define NB_PRIO_QUEUE 3

#include "workers.h"

struct scheduler_ctx {

    /* Child class that contain dictionnary to keep track of harware send and receive I/O */
    alignas(64) Async_transport_ctx tctx;

    /* Child class that contain priority queues and related methods */
    alignas(64) PQueueObj pqueues[NB_PRIO_QUEUE];

    /* Scheduler state */
    _Atomic uint8_t running;

    worker_arg_t thread_args[NB_PRIO_QUEUE];
    pthread_t worker_threads[NB_PRIO_QUEUE];

    reaper_arg_t reap_arg;
    pthread_t reap_thread;

    int worker_ids[NB_PRIO_QUEUE];

    void (*destroy)(Scheduler_ctx *self);
    void (*log_scheduler)(Scheduler_ctx *self);
    void (*start_scheduler)(Scheduler_ctx *self, rnd_bench_ctx_t* bench);

};

Scheduler_ctx * create_scheduler_context(volatile void * bar, Nvmeq_context_t * admin_ctx);


/*


*/
#endif