#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdalign.h>

#include "macros.h"
#include "nvme_sqe.h"
#include "nvme_transport.h"
#include "nvme_queue_ctx.h"
#include "IO_transport_ctx.h"


void _push_cid(Async_transport_ctx * self, uint16_t cid)
{
    uint32_t tail = atomic_load_explicit(&self->tail, memory_order_relaxed);

    self->available_cid[tail & 0xFFFF] = cid;

    atomic_store_explicit(&self->tail, tail + 1, memory_order_release);

}

uint32_t _pop_cid(Async_transport_ctx * self)
{
    uint32_t head = atomic_load_explicit(&self->head, memory_order_relaxed);
    uint32_t tail = atomic_load_explicit(&self->tail, memory_order_acquire);

    if (head == tail) {
        return 0xFFFFFFFF;
    }

    uint32_t cid = self->available_cid[head & 0xFFFF];
    atomic_store_explicit(&self->head, head + 1, memory_order_release);
    return cid;
}


/*
*  Unified interface to update the transport context to ensure a lock free
*  fluid layer
*/
uint8_t _update_requests(Async_transport_ctx *self, uint8_t new_state, uint8_t new_status, uint16_t cid)
{
    uint8_t current = self->TaskTable[cid].state;
    MEM_FENCE(r, rw);
    uint8_t expected;

    /* Special Case: Direct transition from FREE (0) to DONE (2) 
     * allowed only if the task is already expired/invalid.
     */
    if (new_state == STATE_DONE && current == STATE_FREE && new_status == STATUS_DEADLINE_PASSED) {
        expected = STATE_FREE;
    } else {
        expected = (new_state == STATE_FREE) ? STATE_DONE : (new_state - 1);
    }

    /* Update data (status) before the state barrier */
    atomic_store(&self->TaskTable[cid].status, new_status);

    /* Atomic state transition */
    if (!atomic_compare_exchange_strong(&self->TaskTable[cid].state, &expected, new_state)) {
        printf("[CRITICAL] State sync error: CID %u, Expected %u, Target %u\n", 
            cid, expected, new_state);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void tctx_class_init(Async_transport_ctx * tctx)
{
    /* Asynchronous Transport Context Initialization 
       (see IO_transport_ctx.h to get tcx object infos)
    */
    tctx->tail = MAX_REQ_CAP;

    for (uint32_t i = 0; i < MAX_REQ_CAP; i++) {
        tctx->available_cid[i] = i;
    }

    tctx->push_cid = _push_cid;
    tctx->pop_cid = _pop_cid;
    tctx->update_requests = _update_requests;
}