#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "listing.h"
#include "opt_parser.h"
#include "padding-handling.h"
#include "utility.h"

/* On most systems 512 is the default block size
 * It is the norm on most UNIX fs
 */
#ifndef DEFAULT_BLK_SIZE
#define DEFAULT_BLK_SIZE 512
#endif /* !DEFAULT_BLK_SIZE */

/*
 * Global variable that contain all informations of printing
 * Including maximum lengths to adapt the padding during
 * printing
 */
PaddingInfos *PINFOS;

/*
 * Global varaible that contains all the flag set by the user and present
 * in the command line, they will dictate how ls should behave
 */
UsrOptions *USR_OPT;

/*
 * Global variable that will hold the blocksize value, setted once here
 * and used in the byte computations functions
 */
long BLOCKSIZE;

/*
 * Routine that will get the option passed in the command line argument
 *
 * Note: Once a token that is not an option is met, the tokenizing stop
 * 	 Considering each subsequent tokens as targets, even if formated
 * 	 like options
 */
static int
tokenize_cmd(int *argc, char **input[], UsrOptions *usr_opt)
{
    int opt_err = 0;
    int opt_found;

    while ((opt_found = getopt(*argc, *input, "AacdFfhiklnqRrSstuw")) != -1) {
        /* If an option is invalid stop exec and return the error */
        opt_err = OptSet((char)opt_found, usr_opt);
        if (opt_err) {
            throw_error(NULL, WRNG_OPT_ERR);
            return opt_err;
        }
    }
    *argc -= optind;
    *input += optind;
    return EXIT_SUCCESS;
}

int
ls_main(int argc, char *argv[])
{
    /* Set default options for non printable characters and root */
    RootOptionSet(USR_OPT);
    NonPrintableOptionSet(USR_OPT);

    if (tokenize_cmd(&argc, &argv, USR_OPT)) {
        return errno;
    }

    if (!argc) {
        char *dot[] = {".", NULL};
        argv = dot;
        argc = 1;
    }

    getbsize(NULL, &BLOCKSIZE);

    if (TreeTraversal(argc, argv)) {
        return errno;
    }

    return EXIT_SUCCESS;
}

int
main(int argc, char **argv)
{

    int errcode;
    setprogname(argv[0]);

    USR_OPT = calloc(1, sizeof(UsrOptions));
    if (!USR_OPT) {
        throw_error(NULL, MEM_ERR);
        return errno;
    }

    PINFOS = calloc(1, sizeof(PaddingInfos));
    if (!PINFOS) {
        throw_error(NULL, MEM_ERR);
        return errno;
    }

    errcode = ls_main(argc, argv);

    free(USR_OPT);
    free(PINFOS);

    return errcode;
}
