#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "listing.h"
#include "opt_parser.h"
#include "padding-handling.h"
#include "utility.h"

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
 * and used in the byte computations functions, and should not be 
 * changed once the traversal has been called.
 */
long BLOCKSIZE;

/*
 * parse_cmd : Routine that will parse the options passed in the 
 * command line argument by the user
 *
 * Note: Once a token that is not an option is met, the tokenizing stop
 * 	 Considering each subsequent tokens as targets, even if formated
 * 	 like options
 */
static int
parse_cmd(int *argc, char **input[], UsrOptions *usr_opt)
{
    int opt_err = 0;
    int opt_found;

    while ((opt_found = getopt(*argc, *input, "AacdFfhiklnqRrSstuw")) != -1) {
        /* If an option is invalid stop exec and return the error */
        opt_err = OptSet((char)opt_found, usr_opt);
        if (opt_err) {
            warnx("usage : ls [-AacdFfhiklnqRrSstuw] [file ...]");

            return EXIT_FAILURE;
        }
    }
    *argc -= optind;
    *input += optind;
    return EXIT_SUCCESS;
}

/*
 * ls_main : This routine is the entry point of our main ls_routine
 * When calling this function it is absolutely necessary that USR_OPT
 * and PINFOS are alloc-ed and ready to be used.
 *
 * Note: This routine return EXIT_SUCCESS on success or a non null integer
 * representing the error encountered during ls execution.
 * 
 * WARNING: The function can return an error code but output some 
 * valid file listing as long as one target found is valid
 * (depending on the USR_OPT)
 */ 
int
ls_main(int argc, char *argv[])
{
    /* Set default options for non printable characters and root */	
    RootOptionSet(USR_OPT);

    if (parse_cmd(&argc, &argv, USR_OPT)) {
        return EXIT_FAILURE;
    }
    
    OutputOptionSet(USR_OPT);

    /* 
     * If no argument other than options are found during tokenization
     * add the default '.' target.
     */
    if (!argc) {
        char *dot[] = {".", NULL};
        argv = dot;
        argc = 1;
    }

    /* 
     * Retreive BLOCKSIZE environment variable, automatically handle 
     * case where the variable is wrong formated, invalid and will
     * default to the default block size used in the fs
     * if it's the case. see getbsize(3) for more information.
     */
    getbsize(NULL, &BLOCKSIZE);

    if (tree_traversal(argc, argv)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int
main(int argc, char **argv)
{

    int errcode;
    setprogname(argv[0]);


    /* 
     * Allocate both USR_OPT and PINFOS global structures that
     * will contain the options passed by the user and 
     * the printing statistics that padding_handling.c will need.
     *
     * If we do it here, it will be easier for us to avoid leaks.
     */
    USR_OPT = calloc(1, sizeof(UsrOptions));
    if (!USR_OPT) {
        errx(1, "ls: memory error: %s\n", strerror(errno));
        return errno;
    }

    PINFOS = calloc(1, sizeof(PaddingInfos));
    if (!PINFOS) {
        errx(1, "ls: memory error: %s\n", strerror(errno));
        return errno;
    }

    errcode = ls_main(argc, argv);

    free(USR_OPT);
    free(PINFOS);

    return errcode;
}
