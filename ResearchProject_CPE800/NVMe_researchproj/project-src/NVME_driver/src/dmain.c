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

// TODO : remove after IO test
#include "IO_transport_ctx.h"
#include "nvme_transport.h"
#include "nvme_sqe.h"


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

    destroy_nvmeq_ctx(io_ctx, io_ctx->pool_size);
    destroy_nvmeq_ctx(admin_ctx, admin_ctx->pool_size);
}


static int8_t nvme_init_procedure(volatile void * pci_bar, Nvmeq_context_t *admin_ctx, Nvmeq_context_t * io_ctx) {

    L_INFO("Trying to init NVMe context and controller");

    /* Get NVMe registers to init the admin_ctx */
    volatile Nvme_registers * regs = (volatile Nvme_registers *) pci_bar;

    L_INFO("Initializing Queue Context");
    if (nvme_init_ctx(regs, admin_ctx, io_ctx)) {
        driver_exit(pci_bar, admin_ctx, io_ctx);
        return EXIT_FAILURE;
    }

    L_INFO("Enabling NVMe controller");
    if (nvme_enable(regs)) {
        driver_exit(pci_bar, admin_ctx, io_ctx);
        return EXIT_FAILURE;
    }

    L_INFO("Sending Controller I/O Queue creation request");
    if (nvme_io_queue_pair_create(regs, admin_ctx, io_ctx)) {
        driver_exit(pci_bar, admin_ctx, io_ctx);
        return EXIT_FAILURE;
    }

    nvme_cap_log(pci_bar);
    nvme_cc_log(pci_bar);
    nvme_csts_log(pci_bar);
    nvme_cmbloc_log(pci_bar);
    nvme_log_asq_acq(pci_bar);

    L_SUCC("NVMe context, admin queues, I/O queue and controller enabled");

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

    admin_ctx = create_nvmeq_ctx(DEVICE_NVMEQ_BUFF_SIZE, NVME_QUEUE_DEPTH, NVME_QUEUE_DEPTH);
    io_ctx = create_nvmeq_ctx(DEVICE_NVMEQ_BUFF_SIZE, NVME_QUEUE_DEPTH, NVME_QUEUE_DEPTH);

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
        PHASE 1 : SYNCHRONOUS MODE FOR INITIALISATION
        NVMe initialization handshake (step 1/2/3/4)
    */
    if (nvme_init_procedure(pci_bar, admin_ctx, io_ctx)) {
        driver_exit(pci_bar, admin_ctx, io_ctx);
        return EXIT_FAILURE;
    }


    /* PHASE 2 : ASYNCHRONOUS MODE FOR BENCHMARKING */


    /* --- CONFIGURATION DU STRESS TEST --- */
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

    /* 1. ALLOCATION ET PRÉPARATION */
    for (int i = 0; i < STRESS_ITERATIONS; i++) {
        if (i == BIG_REQ_INDEX) {
            /* Cas spécial : Requête de 16 KiB */
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
            /* Requêtes normales : 4 KiB */
            posix_memalign(&buffers[i], 4096, 4096);
            memset(buffers[i], 0xA0 + (i % 0x10), 4096);
        }
    }

    L_INFO("Blasting commands into SQ (Batch mode)...");

    /* 2. SOUMISSION MASSIVE (BATCH) */
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

    /* 3. TRIGGER UNIQUE (DOORBELL) */
    // On ne prévient le contrôleur qu'une seule fois après avoir tout rempli
    __asm__ __volatile__ ("fence w, w" : : : "memory"); 
    nvme_trigger_doorbell(pci_bar, 1, 1, io_ctx->sq_tail);
    
    L_INFO("Doorbell rung. Waiting for completions...");

    /* 4. POLLING DE COMPLÉTION */
    while (tctx->get_completed(tctx) < STRESS_ITERATIONS) {
        IO_receive(io_ctx, tctx);
        // On pourrait ajouter un timeout ici pour l'audit de sécurité
    }

    /* 5. VÉRIFICATION ET NETTOYAGE */
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

    tctx->destroy(tctx);
    
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