#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "tokenize.h"
#include "opt_parser.h"
#include "targ_parser.h"
#include "error.h"


int throw_error(char wrng_opt, char * wrng_targ, char * reason, int err)
{
    switch (err)
    {
        case MEM_ERR:
            fprintf(stderr, "ls: memory error encountered. exiting with error code : %i\n", MEM_ERR);
            return MEM_ERR;
        case WRNG_OPT_ERR:
            fprintf(stderr, "ls: invalid option -- '%c'\nTry 'ls --help' for more information.\nexiting with error code : %i\n", wrng_opt, WRNG_OPT_ERR);
            return WRNG_OPT_ERR;
        case WRNG_TARG_ERR:
            fprintf(stderr, "ls: cannot access '%s': %s\nexiting with error code : %i\n", wrng_targ, reason, WRNG_TARG_ERR);
            return WRNG_TARG_ERR;
        default :
            fprintf(stderr, "ls: something went really wrong..... this should never ever be triggered, it's like opening the pandora box. Never ever use ls like that again !!\n");
            return 666;
    }
}
