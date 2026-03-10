/*
    nvmeq.c : Direct Device Memory access

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

#include "nvme_q.h"


Nvmeq_context_t * init_nvmeq_ctx(uint64_t pool_size) {

    uint64_t page_size = sysconf(_SC_PAGESIZE);

    Nvmeq_context_t * nvmeq_ctx = calloc(1, sizeof(Nvmeq_context_t));
    if (nvmeq_ctx == NULL) {
        warn("Error");
        return NULL;
    }

    int64_t pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap_fd < 0) {
        destroy_nvmeq_ctx(nvmeq_ctx);
        warn("Error");
        return NULL;
    }

    void * nvmeq_buffer = mmap(NULL, pool_size, PROT_READ | PROT_WRITE, 
                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    
                    if (nvmeq_buffer == NULL || nvmeq_buffer == MAP_FAILED) {
        destroy_nvmeq_ctx(nvmeq_ctx);
        warn("Error");
        return NULL;
    }

    mlock(nvmeq_buffer, pool_size);

    nvmeq_ctx->page_size = page_size;
    nvmeq_ctx->pool_size = pool_size;
    nvmeq_ctx->pagemap_fd = pagemap_fd;
    nvmeq_ctx->nvmeq_buff = nvmeq_buffer;

    return nvmeq_ctx;
}

void destroy_nvmeq_ctx(Nvmeq_context_t * nvmeq_ctx) {
    if (nvmeq_ctx->pagemap_fd > 0) {
        close(nvmeq_ctx->pagemap_fd);
    }
    if(nvmeq_ctx && nvmeq_ctx->nvmeq_buff) {
        munlock(nvmeq_ctx->nvmeq_buff, nvmeq_ctx->pool_size);
        munmap(nvmeq_ctx->nvmeq_buff, nvmeq_ctx->pool_size);
    }
    free(nvmeq_ctx);
}

uint64_t nvmeq_to_phys(Nvmeq_context_t * nvmeq_ctx, uint64_t virt_addr) 
{
    uint64_t idx = (virt_addr / nvmeq_ctx->page_size)*sizeof(uint64_t);

    uint64_t pagemap_entry;

    if (pread(nvmeq_ctx->pagemap_fd, 
            &pagemap_entry, 
            sizeof(pagemap_entry),
            idx) == -1) {
        warn("Error");
        destroy_nvmeq_ctx(nvmeq_ctx);
        return EXIT_FAILURE;
    }
    
    if (!IS_PAGE_PRESENT(pagemap_entry)) {
        warnx("Page in RAM is not present");
        destroy_nvmeq_ctx(nvmeq_ctx);
        return EXIT_FAILURE;
    }

    uint64_t phys_pfn = (GET_PFN(pagemap_entry) * nvmeq_ctx->page_size);

    return phys_pfn + (virt_addr % nvmeq_ctx->page_size);
}