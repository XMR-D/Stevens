#ifndef NVME_QUEUE_CONTEXT_H
#define NVME_QUEUE_CONTEXT_H

#include <stdint.h>

typedef struct {
    uint64_t pagemap_fd;      /* File descriptor for /proc/self/pagemap to perform address translation */
    uint64_t page_size;       /* System page size (typically 4096 bytes) */
    uint64_t pool_size;

    uint64_t sq_phys_addr;    /* Physical base address of the Submission Queue (used by the Controller DMA) */
    uint64_t cq_phys_addr;    /* Physical base address of the Completion Queue (used by the Controller DMA) */
    uint64_t sq_depth;        /* Maximum number of entries in the Submission Queue (0-based value in registers) */
    uint64_t cq_depth;        /* Maximum number of entries in the Completion Queue (0-based value in registers) */

    uint64_t sq_virt_addr;    /* Virtual base address of the Submission Queue (used by the CPU to write commands) */
    uint64_t cq_virt_addr;    /* Virtual base address of the Completion Queue (used by the CPU to read responses) */

    void * nvmeq_buff;        /* Pointer to the contiguous memory block allocated via mmap for both queues */

    uint16_t sq_tail;         /* Current write index for the host; updated before ringing the doorbell */
    uint16_t cq_head;         /* Current read index for the host; updated after consuming a completion entry */
    int8_t expected_phase;    /* Expected Phase Tag (P) bit value to identify new completion entries */

} Nvmeq_context_t;


Nvmeq_context_t * create_nvmeq_ctx(uint64_t pool_size, int32_t sq_depth, int32_t cq_depth); 
void destroy_nvmeq_ctx(Nvmeq_context_t * nvmeq_ctx, uint64_t pool_size);

uint64_t nvmeq_to_phys(Nvmeq_context_t * nvmeq_ctx, uint64_t virt_addr);

#define PAGEMAP_PFN_MASK    ((1ULL << 55) - 1)
#define PAGEMAP_PRESENT     (1ULL << 63)
#define PAGEMAP_SWAPPED     (1ULL << 62)


#define GET_PFN(data)       ((data) & PAGEMAP_PFN_MASK)
#define IS_PAGE_PRESENT(data) ((data) & PAGEMAP_PRESENT)

#endif /* !NVME_QUEUE_CONTEXT_H */