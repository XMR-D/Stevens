#ifndef NVME_Q_H
#define NVME_Q_H

#include <stdint.h>

typedef struct {
    uint64_t pagemap_fd;
    uint64_t page_size;
    uint64_t pool_size;
    uint64_t sq_phys_addr;
    uint64_t cq_phys_addr;
    uint64_t sq_virt_addr;
    uint64_t cq_virt_addr;

    void * nvmeq_buff;
} Nvmeq_context_t;


Nvmeq_context_t * create_nvmeq_ctx(uint64_t pool_size);
void destroy_nvmeq_ctx(Nvmeq_context_t * nvmeq_ctx);

uint64_t nvmeq_to_phys(Nvmeq_context_t * nvmeq_ctx, uint64_t virt_addr);

#define PAGEMAP_PFN_MASK    ((1ULL << 55) - 1)
#define PAGEMAP_PRESENT     (1ULL << 63)
#define PAGEMAP_SWAPPED     (1ULL << 62)


#define GET_PFN(data)       ((data) & PAGEMAP_PFN_MASK)
#define IS_PAGE_PRESENT(data) ((data) & PAGEMAP_PRESENT)

#endif /* !NVME_Q_H */