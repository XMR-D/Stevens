
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "options.h"


void print_help(void)
{
    printf("nvme_driver: Deterministic User-Space NVMe Driver with I/O scheduling\n"
           "\n"
           "Deadline-aware and deterministic NVMe driver that implements\n"
           "an Earliest Deadline First (EDF) algorithm at the user-space level.\n" 
           "Unlike traditional drivers, this implementation map NVMe registers\n"
           "directly into user-space. This approach reduces the performance\n"
           "cost induced by context switching and achieves superior latency\n"
           "by shortening the execution \"path\" to the hardware.\n"
           "Finally, workloads are sorted and dispatched based on their specific\n"
           "real-time constraints enabling fine-grained classification and scheduling\n"
           "of commands before they are offloaded to the NVMe hardware.\n"
           "\n"
           "General options:\n"
           "\n"
           "    -h             Print this help.\n"
            "\n\n\n"
            "Usage:");
}

Opt_flgs *
parse_options(int *argc, char ** argv[])
{
    int8_t help = 0;
    uint8_t opt_found = 0;
    optind = 1;
    
    Opt_flgs * opts = calloc(1, sizeof(Opt_flgs));
    if (opts == NULL) {
        warnx("nvme_driver: Error: calloc(%s)\n", strerror(errno));
        return NULL;
    }

    if (*argc == 1) {
        free(opts);
        warnx("Usage: nvme_driver [-h] [nvme_ressource_path] [nvme_bdf]\n");
        return NULL;
    }

    while ((opt_found = getopt(*argc, *argv, "h")) != 0xff) {
        printf("%x\n", opt_found);

        switch (opt_found) {
            case 'h':
                opts->h++;
                break;
            default:
                free(opts);
                warnx("nvme_driver: Error: unrecognized option '-%c'", opt_found);
                return NULL;
        }
    }

    /* Reduce the number of arguments and skip all found options */
    *argc -= optind;
    *argv += optind;

    /*  
        After parsing of command line argument, if more than one was found
        then return with an error.
    */    
    if (*argc != 2) {
        free(opts);
        warnx("nvme_driver: Error: Wrong arguments,"
            "type -h for more informations");
        return NULL;
    }

    return opts;
}
