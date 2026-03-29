#ifndef PRIORITY_QUEUES_CONTEXT_H
#define PRIORITY_QUEUES_CONTEXT_H

#include <stdint.h>
#include <stdatomic.h>
#include <stdalign.h>

#include "nvme_spec.h"
#include "nvme_queue_ctx.h"

#define PQUEUE_CAP 65536

typedef struct task_obj TObj;
typedef struct prio_queue_obj PQueueObj;

struct task_obj {
    uint32_t cid;
    uint64_t timestamp_start;
    uint64_t absolute_deadline;
};

struct prio_queue_obj {

    /* Ring buffer that contains the objects */
    alignas(64) TObj queue[PQUEUE_CAP];
    
    /* Dispatcher access head */
    alignas(64) _Atomic uint32_t head; 

    /* Sender access tail and io_ctx */
    alignas(64) _Atomic uint32_t tail;

    alignas(64) Nvmeq_context_t io_ctx;

    uint8_t nb_in_batch;

    /* Class methods*/

    /* Used by the dispatcher to submit new tasks */
    void (*push_Tobj)(PQueueObj * self, uint16_t cid, uint64_t absolute_deadline, uint64_t timestamp_start);

    /* Used by the sender to retreive a job and send it to the transport layer */
    TObj (*pop_Tobj)(PQueueObj *self);

};

int8_t pqueue_class_init(volatile void * bar, Nvmeq_context_t * admin_ctx, PQueueObj * obj, int64_t pagemap_fd, uint8_t i);

#endif /* PRIORITY_QUEUES_CONTEXT_H */