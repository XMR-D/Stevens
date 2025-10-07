#include "error.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opt_parser.h"

int
throw_error(char *wrng_targ, int err)
{
    switch (err) {
    case MEM_ERR:
        fprintf(stderr, "ls: memory error encountered.\n");
        return errno;
    case WRNG_OPT_ERR:
        fprintf(stderr, "usage: ls [-AacdFfhiklnqRrSstuw] [file...]\n");
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
