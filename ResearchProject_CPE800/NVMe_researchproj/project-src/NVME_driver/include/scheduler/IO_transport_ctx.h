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

    /* Push freed cid in the available cid ring buffer */
    void (*push_cid)(Async_transport_ctx *self, uint16_t cid);

    /* Pop and retreive a free cid in the available cid ring buffer*/
    uint16_t (*pop_cid)(Async_transport_ctx *self);

    /* Update CID status (op=1 for add, op=0 for remove) after queue submission */
    void (*update_requests)(Async_transport_ctx *self, uint16_t cid, uint8_t new_state);

};

void tctx_class_init(Async_transport_ctx * tctx);

#endif /* !IO_TRANSPORT_CONTEXT_H*/