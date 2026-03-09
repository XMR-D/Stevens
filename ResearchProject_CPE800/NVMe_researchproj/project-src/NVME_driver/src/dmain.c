#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "nvme_core.h"
#include "nvme_spec.h"
#include "options.h"
#include "ddma.h"

#include "log.h"


static int8_t driver_main(char * res_path, char * bdf)
{
    ddma_context_t * ddma;
    volatile void * pci_bar;

    /* 
        Create the direct device memory access context required
        to bridge the userspace driver to the MMIO NVMe controller
    */
    L_INFO("Creating DDMA and mapping pci bar register mapped to NVMe device into process space");

    ddma = init_ddma_ctx(DEVICE_DDMA_BUFF_SIZE);
    if (ddma == NULL) {
        return EXIT_FAILURE;
    }

    pci_bar = bar_map(res_path, bdf);
    if (pci_bar == NULL) {
        destroy_ddma_ctx(ddma);
        return EXIT_FAILURE;
    }

    printf("DMA after init ==== :\n");
    printf("pagemap_fd : %ld\n", ddma->pagemap_fd);
    printf("page_size : %ld\n", ddma->page_size);
    printf("pool_size : %ld\n", ddma->pool_size);
    printf("dma_buff : %lx\n", (uint64_t) ddma->ddma_buff);

    printf("BAR0 base addr ==== :\n");
    printf("%lx\n", (uint64_t) pci_bar);

    L_SUCC("DDMA context created successfully");
    L_INFO("Trying to initialized NVMe controler");

    if (nvme_init(pci_bar, ddma)) {
        bar_unmap(pci_bar);
        destroy_ddma_ctx(ddma);
        return EXIT_FAILURE;
    }

    L_INFO("Destroying DDMA context and unmaping pci bar register mapped to NVMe device");
    bar_unmap(pci_bar);
    destroy_ddma_ctx(ddma);
    return EXIT_SUCCESS;
}


int main(int argc, char ** argv) {

    int8_t errcode;
    Opt_flgs * opts = NULL;

    if ((opts = parse_options(&argc, &argv)) == NULL) {
        return EXIT_FAILURE;
    }
    
    if (opts->h) {
        print_help();
        return EXIT_SUCCESS;
    }

    errcode = driver_main(argv[0], argv[1]);
    free(opts);

    return errcode;
}