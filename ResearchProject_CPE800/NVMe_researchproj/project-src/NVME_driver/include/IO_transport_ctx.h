#ifndef IO_TRANSPORT_CONTEXT_H
#define IO_TRANSPORT_CONTEXT_H

#include <stdint.h>
#include <stdatomic.h>
#include <stdalign.h>

#include "nvme_spec.h"

/* define a max request capacity for all the context */
#define MAX_REQ_CAP NVME_MAX_CID

/* ASYNCHRONOUS TRANSPORT CONTEXT STRUCTURE (SHARED) */

typedef struct async_transport_ctx Async_transport_ctx;

struct async_transport_ctx {

    /* Lock-free array storing the status of each CID currently in flight */
    alignas(64) _Atomic uint8_t request_status[MAX_REQ_CAP];

    /* Lock-free ring buffer to store available cids */
    alignas(64) _Atomic uint16_t available_cid[MAX_REQ_CAP];
    alignas(64) _Atomic uint32_t head; 
    alignas(64) _Atomic uint32_t tail;

    /* Atomic counter tracking the total number of pending asynchronous tasks */
    alignas(64) _Atomic uint16_t nb_pending_requests;

    /* Atomic counter tracking the total number of completed asynchronous tasks */
    alignas(64) _Atomic uint16_t nb_completed_requests;

    /* Check if a CID from a priority queue is currently registered as active */
    uint8_t (*is_active)(Async_transport_ctx *self, uint16_t cid);
    
    /* Return the total count of requests currently managed by the transport layer */
    uint16_t (*get_pending)(Async_transport_ctx *self);

/* Return the total count of requests currently managed by the transport layer */
    uint16_t (*get_completed)(Async_transport_ctx *self);

    /* Update CID status (op=1 for add, op=0 for remove) after queue submission */
    uint16_t (*update_requests)(Async_transport_ctx *self, uint16_t cid, uint8_t new_state);

    /* Destroy and free the object */
    void (*destroy)(Async_transport_ctx * self);

};

Async_transport_ctx * create_asynch_transport_context(void);

#endif /* !IO_TRANSPORT_CONTEXT_H*/