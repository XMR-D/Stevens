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
#include "nvme_core.h"
#include "nvme_spec.h"
#include "options.h"
#include "nvme_queue_ctx.h"


#include "scheduler_ctx.h"

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
    L_INFO("Attempting to create the scheduler context");
    Scheduler_ctx * scheduler = create_scheduler_context(pci_bar, admin_ctx);
    if (scheduler == NULL) {
        L_ERR("Failed to create the scheduler context", "NVMe Timeout");
        return EXIT_FAILURE;
    }

    L_SUCC("Scheduler context created");
    scheduler->log_scheduler(scheduler);


    driver_exit(pci_bar, admin_ctx, scheduler);
    return EXIT_SUCCESS;


    /* --- CONFIGURATION DU STRESS TEST --- */

    /*
    #define STRESS_ITERATIONS 127
    #define LBA_START 2000          // On commence loin pour être tranquille
    #define BIG_REQ_INDEX 10        // L'indice de la "grosse" insertion
    #define BIG_REQ_PAGES 4         // 16 KiB (4 * 4096)
    #define LBA_STEP 10             // Pas entre chaque itération pour éviter l'overlap

    Async_transport_ctx *tctx = create_asynch_transport_context();
    int32_t cids[STRESS_ITERATIONS];
    void *buffers[STRESS_ITERATIONS];
    
    // Variables pour la requête multi-page
    void *big_buffer = NULL;
    uint64_t *prp_list = NULL;
    uint64_t big_prp1 = 0;
    uint64_t big_prp2 = 0;

    L_INFO("Allocating buffers and preparing PRP List...");

    for (int i = 0; i < STRESS_ITERATIONS; i++) {
        if (i == BIG_REQ_INDEX) {
            posix_memalign(&big_buffer, 4096, 4096 * BIG_REQ_PAGES);
            posix_memalign((void**)&prp_list, 4096, 4096);
            
            memset(big_buffer, 0x42, 4096 * BIG_REQ_PAGES); // Pattern "B"
            memset(prp_list, 0, 4096);

            // PRP1 pointe sur la première page
            big_prp1 = nvmeq_to_phys(io_ctx, (uint64_t)big_buffer);
            
            // La PRP List contient les adresses des pages 2, 3 et 4
            for (int p = 1; p < BIG_REQ_PAGES; p++) {
                prp_list[p-1] = nvmeq_to_phys(io_ctx, (uint64_t)big_buffer + (p * 4096));
            }
            // PRP2 pointe sur la PRP List elle-même
            big_prp2 = nvmeq_to_phys(io_ctx, (uint64_t)prp_list);
            
            buffers[i] = big_buffer;
        } else {
            posix_memalign(&buffers[i], 4096, 4096);
            memset(buffers[i], 0xA0 + (i % 0x10), 4096);
        }
    }

    L_INFO("Blasting commands into SQ (Batch mode)...");

    for (int i = 0; i < STRESS_ITERATIONS; i++) {
        uint64_t p1, p2;
        uint32_t nlb;
        uint64_t current_lba = LBA_START + (i * LBA_STEP);

        if (i == BIG_REQ_INDEX) {
            p1 = big_prp1;
            p2 = big_prp2;
            nlb = BIG_REQ_PAGES; // Le formateur fera (4 - 1) = 3
        } else {
            p1 = nvmeq_to_phys(io_ctx, (uint64_t)buffers[i]);
            p2 = 0;
            nlb = 1;
        }

        // Appel direct au transporteur
        cids[i] = IO_send(io_ctx, tctx, NVME_IO_WRITE, 1, current_lba, nlb, p1, p2);

        if (cids[i] == -1) {
            L_ERR("STRESS", "Submission failed at index (Queue Full)");
            break;
        }
    }

    // On ne prévient le contrôleur qu'une seule fois après avoir tout rempli
    __asm__ __volatile__ ("fence w, w" : : : "memory"); 
    nvme_trigger_doorbell(pci_bar, 1, 1, io_ctx->sq_tail);
    
    L_INFO("Doorbell rung. Waiting for completions...");

    while (tctx->get_completed(tctx) < STRESS_ITERATIONS) {
        IO_receive(io_ctx, tctx);
        // On pourrait ajouter un timeout ici pour l'audit de sécurité
    }

    int failed_status = 0;
    for (int i = 0; i < STRESS_ITERATIONS; i++) {
        if (tctx->is_active(tctx, cids[i]) != 2) {
            failed_status++;
        }
        
        // Libération du CID dans le dictionnaire
        tctx->update_requests(tctx, cids[i], 0);
        
        // Libération mémoire
        if (i == BIG_REQ_INDEX) {
            free(big_buffer);
            free(prp_list);
        } else {
            free(buffers[i]);
        }
    }

    if (failed_status == 0) {
        L_INFO("SUCCESS: All commands (including 16KB) processed.");
    } else {
        L_ERR("STRESS", "FAILURE: commands did not reach status 'Completed'");
    }

    tctx->destroy(tctx); */
    
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