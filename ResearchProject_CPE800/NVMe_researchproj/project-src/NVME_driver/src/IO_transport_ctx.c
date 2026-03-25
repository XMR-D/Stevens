#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "macros.h"

#include "IO_transport_ctx.h"


static void _push_cid(Async_transport_ctx * self, uint16_t cid)
{
    uint32_t tail = atomic_load_explicit(&self->tail, memory_order_relaxed);

    self->available_cid[tail & 0xFFFF] = cid;

    atomic_store_explicit(&self->tail, tail + 1, memory_order_release);

}

static uint16_t _pop_cid(Async_transport_ctx * self)
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

/* 1 means pending */
uint8_t _is_active(Async_transport_ctx * self, uint16_t cid)
{
    return atomic_load(&self->request_status[cid]);
}

uint16_t _get_pending(Async_transport_ctx * self)
{
    return atomic_load(&self->nb_pending_requests);
}

uint16_t _get_completed(Async_transport_ctx * self)
{
    return atomic_load(&self->nb_completed_requests);
}

/*
   Unified interface to update the transport context to ensure a lock free
   fluid SPSC asynchronous layer.

   return allocated cid for pending state
   return argument passed cid otherwise

   on errors : return 0xFFFF if no ressources are available

   state = 0 => free
   state = 1 => pending (submitted to hardware)
   state = 2 => completed (ready for reaper/scheduler)
*/
uint16_t _update_requests(Async_transport_ctx *self, uint16_t cid, uint8_t new_state)
{
    uint8_t expected;

    switch (new_state) {
        /* transition from free to pending state  */
        case 1:
            /* retreive next available cid */
            uint16_t new_cid = _pop_cid(self);
            if (new_cid == 0xFFFF) {
                return 0xFFFF;
            }

            /* change state of the retreived cid and update pending jobs */
            expected = 0;
            if (atomic_compare_exchange_strong(&self->request_status[new_cid], &expected, 1)) {
                atomic_fetch_add(&self->nb_pending_requests, 1);
                return new_cid;
            }
            return 0xFFFF;

        /* transition from pending to completed state  */
        case 2:
            expected = 1;
            if (atomic_compare_exchange_strong(&self->request_status[cid], &expected, 2)) {
                atomic_fetch_sub(&self->nb_pending_requests, 1);
                atomic_fetch_add(&self->nb_completed_requests, 1);

            }
            break;

        /* transition from completed to free state  */
        case 0:
            expected = 2;
            if (atomic_compare_exchange_strong(&self->request_status[cid], &expected, 0)) {
                _push_cid(self, cid);
                atomic_fetch_sub(&self->nb_completed_requests, 1);
            }
            break;

        default:
            break;
    }
    return cid;
}

/* Transport SPSC context destructor */
void _destroy(Async_transport_ctx * self)
{
    if (self) {
        free(self);
    }
}

/* 
   Transport SPSC context constructor.
   
   Note: PLEASE USE A FENCE AFTER CALLING THE FUNCTION TO ENSURE THE STRUCTURE IS CORRECTLY 
   INSTANCIATED AND THREADS DO NOT START OPERATIONS ON A ILL CONTEXT
*/
Async_transport_ctx * create_asynch_transport_context(void)
{
    Async_transport_ctx * obj = calloc(1, sizeof(Async_transport_ctx)); 
    if (!obj) {
        L_ERR("Failed to create asyncronous transport context (see async_transport_dsa.c l.127)", "calloc failed");
        return NULL;
    }

    obj->tail = MAX_REQ_CAP;

    for (uint32_t i = 0; i < MAX_REQ_CAP; i++) {
        obj->available_cid[i] = i;
    }

    obj->is_active = _is_active;
    obj->get_pending = _get_pending;
    obj->get_completed = _get_completed;
    obj->update_requests = _update_requests;
    obj->destroy = _destroy;

    return obj;

}
