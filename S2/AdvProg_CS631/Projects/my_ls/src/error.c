#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "tokenize.h"
#include "opt_parser.h"
#include "targ_parser.h"
#include "error.h"

//TODO: Maybe need to set errno to appropriate error code
//TODO: Need to adapt the reason to why the file cannot be opened (not here)
//TODO: Create a RETURN CODE variable and edit it if an error is thrown
//      Throw the last error that occured
//      Rework the code to match ls(2) descrp and errno err codes
int throw_error(char wrng_opt, char * wrng_targ, char * reason, int err)
{
    switch (err)
    {
        case MEM_ERR:
            fprintf(stderr, "ls: memory error encountered.\n");
            return MEM_ERR;
        case WRNG_OPT_ERR:
            fprintf(stderr, "ls: invalid option -- '%c'\nTry 'ls --help' for more information.\n", wrng_opt);
            return WRNG_OPT_ERR;
        case WRNG_TARG_ERR:
            fprintf(stderr, "ls: cannot access '%s': %s\n", wrng_targ, reason);
            return WRNG_TARG_ERR;
        default :
            /* SHOULD NOT BE EXECUTED AT ANYTIME*/
            fprintf(stderr, "ls: something went really wrong..... this should never ever be triggered, it's like opening the pandora box. Never ever use ls like that again !!\n");
            return 666;
    }
}
