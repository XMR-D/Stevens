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

#include "macros.h"
#include "benchmark.h"
#include "riscv_time.h"

#include "nvme_core.h"
#include "nvme_spec.h"
#include "options.h"
#include "nvme_queue_ctx.h"


#include "scheduler_ctx.h"

/* declare it as static to keep it in bss section and avoid immediate segfault */
static rnd_bench_ctx_t * b_ctx;

static inline void driver_exit(volatile void * pci_bar, Nvmeq_context_t *admin_ctx, Scheduler_ctx * sctx)
{
    L_INFO("Destroying NVMe contexts and unmaping pci bar register mapped to NVMe device");

    if (pci_bar) {
        bar_unmap(pci_bar);
    }

    if (admin_ctx) {
        destroy_nvmeq_ctx(admin_ctx, admin_ctx->pool_size);
    }

    if (sctx) {
        sctx->destroy(sctx);
    }
    L_SUCC("All structures freed");
}


static int8_t driver_enter(char * res_path, char * bdf)
{
    Nvmeq_context_t * admin_ctx = NULL;
    volatile void * pci_bar = NULL;

    /* 
        Create the NVMe queue context and mapping the pci, that way in userspace
        we will be able to communicate using the queues and the pci to the NVMe controller
    */
    L_INFO("Creating NVMe contexts and mapping pci bar register mapped to NVMe device into process space");

    admin_ctx = create_nvmeq_ctx(DEVICE_NVMEQ_BUFF_SIZE, NVME_QUEUE_DEPTH, NVME_QUEUE_DEPTH);

    if (admin_ctx == NULL) {
        driver_exit(pci_bar, admin_ctx, NULL);
        return EXIT_FAILURE;
    }

    pci_bar = bar_map(res_path, bdf);
    if (pci_bar == NULL) {
        driver_exit(pci_bar, admin_ctx, NULL);
        return EXIT_FAILURE;
    }

    /*
        PHASE 1 : SYNCHRONOUS MODE FOR INITIALISATION
        NVMe initialization handshake (step 1/2)
    */
    if (nvme_init_handshake(pci_bar, admin_ctx) == EXIT_FAILURE) {
        driver_exit(pci_bar, admin_ctx, NULL);
        return EXIT_FAILURE;
    }
    L_SUCC("NVMe context created successfully");
    L_SUCC("Init handshake done");


    /* PHASE 2 : ASYNCHRONOUS MODE FOR BENCHMARKING */
    L_INFO("Attempting to generate benchmark....");
    b_ctx = calloc(1, sizeof(rnd_bench_ctx_t));

    if (b_ctx == NULL) {
        L_ERR("Benchmark", "Null benchmark exiting.");
        driver_exit(pci_bar, admin_ctx, NULL);
        return EXIT_SUCCESS;
    }
    b_ctx->cpu_freq_mhz = 10;
    b_ctx->max_requests = NB_WORLOADS;
    b_ctx->read_ratio = 30;
    b_ctx->seed = 0xdeadbeef;

    printf("bench = %p\n", b_ctx);


    L_INFO("Attempting to create the scheduler context");
    Scheduler_ctx * scheduler = create_scheduler_context(pci_bar, admin_ctx);
    if (scheduler == NULL) {
        L_ERR("Failed to create the scheduler context", "NVMe Timeout");
        return EXIT_FAILURE;
    }

    L_SUCC("Scheduler context created");
    //scheduler->log_scheduler(scheduler);

    generate_workload_buffer(b_ctx);
    printf("bench post workload buffer init = %p\n", b_ctx);

    scheduler->start_scheduler(scheduler, b_ctx);

    driver_exit(pci_bar, admin_ctx, scheduler);
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