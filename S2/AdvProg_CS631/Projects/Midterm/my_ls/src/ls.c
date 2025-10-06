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
PaddingInfos * PINFOS;

UsrOptions * USR_OPT;

/* Global variable indicating the default block_size ls must take */
int BLOCKSIZE = DEFAULT_BLK_SIZE;

int 
tokenize(int *argc, char ** input[], UsrOptions *usr_opt)
{
    int opt_err = 0;
    int opt_found;

    while((opt_found = getopt(*argc, *input, "AacdFfhiklnqRrSstuw")) != -1) {
	/* If an option is invalid stop exec and return the error */
	opt_err = OptSet((char) opt_found, usr_opt);
        if (opt_err) {
	    throw_error(NULL, WRNG_OPT_ERR);
            return opt_err;
	}
	*argc -= optind;
	*input += optind;
    }
    return EXIT_SUCCESS;
}

int 
ls_main(int argc, char * argv[])
{
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


	/* Set default options for non printable characters and root */
	RootOptionSet(USR_OPT);
	NonPrintableOptionSet(USR_OPT);

	if (tokenize(&argc, &argv, USR_OPT)) {
		free(USR_OPT);
		free(PINFOS);
		return errno;
	}

	if (!argc) {
		char * dot[] = {".", NULL};
		argv = dot;
		argc = 1;
	}
	
	/* Get block size environment value and check for validity */
	if (USR_OPT->s) {
	    BLOCKSIZE = GetBlockSize();
	}

	if (TreeTraversal(argc, argv)) {
		free(USR_OPT);
		free(PINFOS);
		return errno;
	}

	
	free(USR_OPT);
	free(PINFOS);	
	return EXIT_SUCCESS;
}

int 
main(int argc, char ** argv)
{
    return ls_main(argc, argv);	
}


