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
uint8_t _update_requests(Async_transport_ctx *self, uint8_t expected, uint8_t next, uint8_t status, uint16_t cid)
{
    /* Update status before state transition */
    atomic_store_explicit(&self->TaskTable[cid].status, status, memory_order_relaxed);

    /* Atomic CAS: handles spurious failures and ensures memory visibility */
    while (!atomic_compare_exchange_strong_explicit(&self->TaskTable[cid].state, 
                                                   &expected, next, 
                                                   memory_order_acq_rel, 
                                                   memory_order_acquire)) {
        /* If state changed under our feet, it's a real sync error */
        if (expected != 0 && expected != 1 && expected != 2) { // Logic check
            printf("[CRITICAL] Sync Error: CID %u, State was %u\n", cid, expected);
            return EXIT_FAILURE;
        }
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