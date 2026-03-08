/*
    ddma.c : Direct Device Memory access

    Note :

    Side effect : 

*/
#include <err.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
       
#include <sys/mman.h>

#include "ddma.h"


ddma_context_t * init_ddma_ctx(uint64_t pool_size) {

    uint64_t page_size = sysconf(_SC_PAGESIZE);

    ddma_context_t * ddma_ctx = calloc(1, sizeof(ddma_context_t));
    if (ddma_ctx == NULL) {
        warn("Error");
        return NULL;
    }

    uint64_t pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap_fd < 0) {
        destroy_ddma_ctx(ddma_ctx);
        warn("Error");
        return NULL;
    }

    void * ddma_buffer = mmap(NULL, pool_size, PROT_READ | PROT_WRITE, 
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ddma_buffer == NULL) {
        destroy_ddma_ctx(ddma_ctx);
        warn("Error");
        return NULL;
    }

    mlock(ddma_buffer, pool_size);

    ddma_ctx->page_size = page_size;
    ddma_ctx->pool_size = pool_size;
    ddma_ctx->pagemap_fd = pagemap_fd;
    ddma_ctx->ddma_buff = ddma_buffer;
      
    return ddma_ctx;
}

void destroy_ddma_ctx(ddma_context_t * ddma_ctx) {
    if (ddma_ctx->pagemap_fd > 0) {
        close(ddma_ctx->pagemap_fd);
    }
    if(ddma_ctx && ddma_ctx->ddma_buff) {
        munlock(ddma_ctx->ddma_buff, ddma_ctx->pool_size);
        munmap(ddma_ctx->ddma_buff, ddma_ctx->pool_size);
    }
    free(ddma_ctx);
}

uint64_t ddma_to_phys(ddma_context_t * ddma_ctx, uint64_t virt_addr) 
{
    uint64_t idx = (virt_addr / ddma_ctx->page_size)*sizeof(uint64_t);

    uint64_t pagemap_entry;

    if (pread(ddma_ctx->pagemap_fd, 
            &pagemap_entry, 
            sizeof(pagemap_entry),
            idx) == -1) {
        warn("Error");
        destroy_ddma_ctx(ddma_ctx);
        return 0;
    }
    
    if (!IS_PAGE_PRESENT(pagemap_entry)) {
        warnx("Page in RAM is not present");
        destroy_ddma_ctx(ddma_ctx);
        return 0;
    }

    uint64_t phys_pfn = (GET_PFN(pagemap_entry) * ddma_ctx->page_size);

    return phys_pfn + (virt_addr % ddma_ctx->page_size);
}