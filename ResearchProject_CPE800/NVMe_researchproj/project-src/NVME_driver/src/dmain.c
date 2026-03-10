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
#include "nvme_q.h"

#include "log.h"

/* INFO : The idea for the whole implementaion will be : 
 * Make it work, make it right, then make it fast
 */

/* INFO: The idea will be to have two mode of command submission after the 
scheduling, a batching mode and a quick receive send mode, it will optimized the 
operations regarding onpeak / offpeak situations */

/* INFO: Every sqes will be temporary allocated objects so that handling them is easier
 * It will be this at the first stage, then a faster mode would be implemented where 
 * It will change the creation api to directly write in memory commands.
 *
 */


static int8_t driver_main(char * res_path, char * bdf)
{
    Nvmeq_context_t * nvmeq;
    volatile void * pci_bar;

    /* 
        Create the direct device memory access context required
        to bridge the userspace driver to the MMIO NVMe controller
    */
    L_INFO("Creating nvmeq and mapping pci bar register mapped to NVMe device into process space");

    nvmeq = init_nvmeq_ctx(DEVICE_NVMEQ_BUFF_SIZE);
    if (nvmeq == NULL) {
        return EXIT_FAILURE;
    }

    pci_bar = bar_map(res_path, bdf);
    if (pci_bar == NULL) {
        destroy_nvmeq_ctx(nvmeq);
        return EXIT_FAILURE;
    }

    printf("BAR0 base addr ==== :\n");
    printf("%lx\n", (uint64_t) pci_bar);

    L_SUCC("nvmeq context created successfully");
    L_INFO("Trying to initialized NVMe controler");

    if (nvme_init(pci_bar, nvmeq)) {
        bar_unmap(pci_bar);
        destroy_nvmeq_ctx(nvmeq);
        return EXIT_FAILURE;
    }

    L_INFO("Destroying nvmeq context and unmaping pci bar register mapped to NVMe device");
    bar_unmap(pci_bar);
    destroy_nvmeq_ctx(nvmeq);
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