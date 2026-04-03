#include <err.h>
#include <errno.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/mman.h>

#include "macros.h"

#include "nvme_queue_ctx.h"
#include "nvme_core.h"

#include "scheduler_ctx.h"
#include "IO_transport_ctx.h"

#include "priority_queues_ctx.h"


void _push_Tobj(PQueueObj * self, uint16_t cid, uint64_t absolute_deadline, uint64_t timestamp_start)
{
    uint32_t tail = atomic_load_explicit(&self->tail, memory_order_relaxed);
    TObj tobject;
    tobject.cid = cid;
    tobject.absolute_deadline = absolute_deadline;
    tobject.timestamp_start = timestamp_start;

    self->queue[tail & 0xFFFF] = tobject;

    atomic_store_explicit(&self->tail, tail + 1, memory_order_release);

}

TObj _pop_Tobj(PQueueObj * self)
{
    uint32_t head = atomic_load_explicit(&self->head, memory_order_relaxed);
    uint32_t tail = atomic_load_explicit(&self->tail, memory_order_acquire);

    if (head == tail) {
        TObj tobject;
        tobject.cid = 0xFFFFFFFF;
        return tobject;
    }

    TObj tobject = self->queue[head & 0xFFFF];
    atomic_store_explicit(&self->head, head + 1, memory_order_release);
    return tobject;
}

int8_t pqueue_class_init(volatile void * bar, Nvmeq_context_t * admin_ctx, PQueueObj * obj, int64_t pagemap_fd, uint8_t i) 
{
    /* Priority Queues Context initialisation */
    volatile Nvme_registers * regs = (volatile Nvme_registers *) bar;

    void * nvmeq_buffer = mmap(NULL, DEVICE_NVMEQ_BUFF_SIZE, PROT_READ | PROT_WRITE, 
                MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (nvmeq_buffer == NULL || nvmeq_buffer == MAP_FAILED) {
        return EXIT_FAILURE;
    }

    mlock(nvmeq_buffer, DEVICE_NVMEQ_BUFF_SIZE);
    obj->io_ctx.pagemap_fd = pagemap_fd;
    obj->io_ctx.pool_size= DEVICE_NVMEQ_BUFF_SIZE;
    obj->io_ctx.page_size = sysconf(_SC_PAGESIZE);
    obj->io_ctx.sq_depth = NVME_QUEUE_DEPTH;
    obj->io_ctx.cq_depth = NVME_QUEUE_DEPTH;
    obj->io_ctx.nvmeq_buff = nvmeq_buffer;

    obj->service_time = 0;
    
    if (nvme_init_queue_ctx(&(obj->io_ctx), regs, i, 0) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    /* create the io_queue here */
    if (nvme_ioqueue_create(bar, admin_ctx, &(obj->io_ctx), i) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    obj->push_Tobj = _push_Tobj;
    obj->pop_Tobj = _pop_Tobj;

    return EXIT_SUCCESS;
    
}
