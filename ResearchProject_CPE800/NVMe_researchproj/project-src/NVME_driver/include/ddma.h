#ifndef DDMA_H
#define DDMA_H

#include <stdint.h>

#define DEVICE_MEMBUFFER 

typedef struct {
    uint64_t pagemap_fd;
    uint64_t page_size;
    uint64_t pool_size;
    void * ddma_buff;
} ddma_context_t;


ddma_context_t * init_ddma_ctx(uint64_t pool_size);
void destroy_ddma_ctx(ddma_context_t * ddma_ctx);

uint64_t from_virt(void *virtual_addr);

#define PAGEMAP_PFN_MASK    ((1ULL << 55) - 1)
#define PAGEMAP_PRESENT     (1ULL << 63)
#define PAGEMAP_SWAPPED     (1ULL << 62)


#define GET_PFN(data)       ((data) & PAGEMAP_PFN_MASK)
#define IS_PAGE_PRESENT(data) ((data) & PAGEMAP_PRESENT)

#endif /* !DDMA_H */