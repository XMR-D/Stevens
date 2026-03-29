#ifndef IO_TRANSPORT_CONTEXT_H
#define IO_TRANSPORT_CONTEXT_H

#include <stdint.h>
#include <stdatomic.h>
#include <stdalign.h>

#include "nvme_spec.h"

/* define a max request capacity for the transport layer, normaly no need to change it
as NVMe spec is specifying 16 bits for the cid  */
#define MAX_REQ_CAP 65536

/* ASYNCHRONOUS TRANSPORT CONTEXT STRUCTURE (SHARED) */

typedef struct async_transport_ctx Async_transport_ctx;

#define STATE_FREE      0
#define STATE_PENDING   1
#define STATE_DONE      2

#define STATUS_SUCCESS              0
#define STATUS_PENDING               1
#define STATUS_ERR_NVME             2
#define STATUS_DEADLINE_PASSED      3

typedef struct {
    /* Metadata fields for a NVMe IO command */
    _Atomic uint64_t slba;
    _Atomic uint64_t prp1;
    _Atomic uint64_t prp2;
    _Atomic uint32_t nsid;
    _Atomic uint16_t nlb;
    _Atomic uint8_t opc;

    /* Status and state of the task */
    _Atomic uint8_t state;          
    _Atomic uint8_t status;         

    /* Additional infos for rescheduling */
    _Atomic uint64_t absolute_deadline;
    _Atomic uint64_t timestamp_start;
    _Atomic uint64_t queue_ID;

    /* padding for cache optimization */
    uint8_t pad[15];

}__attribute__((aligned(64))) IO_metadata_t;

struct async_transport_ctx {


    /* Lock-free array storing the status of each CID currently in flight */
    alignas(64) IO_metadata_t TaskTable[MAX_REQ_CAP];

    /* Lock-free ring buffer to store available cids */
    alignas(64) _Atomic uint32_t available_cid[MAX_REQ_CAP];
    alignas(64) _Atomic uint32_t head; 
    alignas(64) _Atomic uint32_t tail;

    /* Push freed cid in the available cid ring buffer */
    void (*push_cid)(Async_transport_ctx *self, uint16_t cid);

    /* Pop and retreive a free cid in the available cid ring buffer*/
    uint32_t (*pop_cid)(Async_transport_ctx *self);

    /* Update CID status (op=1 for add, op=0 for remove) after queue submission */
    uint8_t (*update_requests)(Async_transport_ctx *self, uint8_t new_state, uint8_t new_status, uint16_t cid);

};

void tctx_class_init(Async_transport_ctx * tctx);

#endif /* !IO_TRANSPORT_CONTEXT_H*/