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

uint16_t _pop_cid(Async_transport_ctx * self)
{
    uint32_t head = atomic_load_explicit(&self->head, memory_order_relaxed);
    uint32_t tail = atomic_load_explicit(&self->tail, memory_order_acquire);

    if (head == tail) {
        return 0xFFFF;
    }

    uint16_t cid = self->available_cid[head & 0xFFFF];
    atomic_store_explicit(&self->head, head + 1, memory_order_release);
    return cid;
}

/*
   Unified interface to update the transport context to ensure a lock free
   fluid PMC asynchronous layer.

   Note :

   upon task creation (0->1), cid parameter will be ignored

   return allocated cid for pending state
   return argument passed cid otherwise

   on errors : return 0xFFFF if no ressources are available

   state = 0 => free
   state = 1 => pending (submitted to hardware)
   state = 2 => completed (ready for reaper/scheduler)
*/
void _update_requests(Async_transport_ctx *self, uint16_t cid, uint8_t new_state)
{
    uint8_t expected;

    switch (new_state) {
        /* transition from free to pending state  */
        case 1:
            /* change state of the retreived cid and update pending jobs */
            expected = 0;
            atomic_compare_exchange_strong(&self->request_status[cid], &expected, 1);
            break;

        /* transition from pending to completed state  */
        case 2:
            expected = 1;
            atomic_compare_exchange_strong(&self->request_status[cid], &expected, 2);
            break;

        /* transition from completed to free state  */
        case 0:
            expected = 2;
            if (atomic_compare_exchange_strong(&self->request_status[cid], &expected, 0)) {
                _push_cid(self, cid);
            }
            break;

        default:
            break;
    }
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