#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "log.h"
#include "opt-parser.h"
#include "targ-parser.h"
#include "tokenize.h"

int throw_error(char wrng_opt, char* wrng_targ, int err) 
{
    switch (err) {
        case MEM_ERR:
            fprintf(stderr, "ls: memory error encountered.\n");
            return errno;
        case WRNG_OPT_ERR:
            fprintf(stderr,
                    "ls: invalid option -- '%c'\nTry 'ls --help' for more "
                    "information.\n",
                    wrng_opt);
            return errno;
        case WRNG_TARG_ERR:
            fprintf(stderr, "ls: cannot access '%s': %s\n", wrng_targ,
                    strerror(errno));
            return errno;
        default:
            /* SHOULD NOT BE EXECUTED AT ANYTIME*/
            return errno;
    }
}