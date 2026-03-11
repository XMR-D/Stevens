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
 * scheduling, a batching mode and a quick receive send mode, it will optimized the 
 * operations regarding onpeak / offpeak situations 
 */

/* INFO: Every sqes will be temporary allocated objects so that handling them is easier
 * It will be this at the first stage, then a faster mode would be implemented where 
 * It will change the creation api to directly write in memory commands.
 *
 */


static inline void driver_exit(volatile void * pci_bar, Nvmeq_context_t *admin_ctx, Nvmeq_context_t * io_ctx)
{
    L_INFO("Destroying NVMe contexts and unmaping pci bar register mapped to NVMe device");

    if (pci_bar != NULL) {
        bar_unmap(pci_bar);
    }

    destroy_nvmeq_ctx(io_ctx);
    destroy_nvmeq_ctx(admin_ctx);
}


static int8_t nvme_init_handshake(volatile void * pci_bar, Nvmeq_context_t *admin_ctx, Nvmeq_context_t * io_ctx) {

    L_INFO("Trying to init NVMe context and controller");

    /* Get NVMe registers to init the admin_ctx */
    volatile Nvme_registers * regs = (volatile Nvme_registers *) pci_bar;

    if (nvme_init_ctx(regs, admin_ctx, io_ctx)) {
        driver_exit(pci_bar, admin_ctx, io_ctx);
        return EXIT_FAILURE;
    }
    if (nvme_enable(regs)) {
        driver_exit(pci_bar, admin_ctx, io_ctx);
        return EXIT_FAILURE;
    }

    /* 
    if (nvme_io_configure(regs, admin_ctx, io_ctx)) {
        driver_exit(pci_bar, admin_ctx, io_ctx);
        return EXIT_FAILURE;
    } 
    */

    nvme_cap_log(pci_bar);
    nvme_cc_log(pci_bar);
    nvme_csts_log(pci_bar);
    nvme_cmbloc_log(pci_bar);
    nvme_log_asq_acq(pci_bar);

    L_SUCC("NVMe context initialized successfully and NVMe controller enabled");

    return EXIT_SUCCESS;
}


static int8_t driver_enter(char * res_path, char * bdf)
{
    Nvmeq_context_t * admin_ctx = NULL;
    Nvmeq_context_t * io_ctx = NULL;
    volatile void * pci_bar = NULL;

    /* 
        Create the NVMe queue context and mapping the pci, that way in userspace
        we will be able to communicate using the queues and the pci to the NVMe controller
    */
    L_INFO("Creating NVMe contexts and mapping pci bar register mapped to NVMe device into process space");

    admin_ctx = create_nvmeq_ctx(DEVICE_NVMEQ_BUFF_SIZE);
    io_ctx = create_nvmeq_ctx(DEVICE_NVMEQ_BUFF_SIZE);

    if (admin_ctx == NULL || io_ctx == NULL) {
        driver_exit(pci_bar, admin_ctx, io_ctx);
        return EXIT_FAILURE;
    }

    pci_bar = bar_map(res_path, bdf);
    if (pci_bar == NULL) {
        driver_exit(pci_bar, admin_ctx, io_ctx);
        return EXIT_FAILURE;
    }

    L_SUCC("NVMe context created successfully");

    /*
        NVMe initialization handshake (step 1/2/3/4)
    */
    if (nvme_init_handshake(pci_bar, admin_ctx, io_ctx)) {
        driver_exit(pci_bar, admin_ctx, io_ctx);
        return EXIT_FAILURE;
    }


    driver_exit(pci_bar, admin_ctx, io_ctx);
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

    errcode = driver_enter(argv[0], argv[1]);
    free(opts);

    return errcode;
}