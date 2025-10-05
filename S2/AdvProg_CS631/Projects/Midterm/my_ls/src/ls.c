#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "listing.h"
#include "list-handling.h"
#include "opt_parser.h"
#include "targ_parser.h"
#include "tokenize.h"
#include "utility.h"

/* On most systems 512 is the default block size
 * It is the norm on most UNIX fs
 */
#ifndef DEFAULT_BLK_SIZE
#define DEFAULT_BLK_SIZE 512
#endif /* !DEFAULT_BLK_SIZE */

/* Global variable representing options structure */
UsrOptions * usr_opt;

/* 
 * Global variable that contain all informations of printing
 * Including maximum lengths to adapt the padding during 
 * printing
 */
PrintInfos * PINFOS;

/* Global variable representing if a target as been tested */
int targ_found;

/* Global variable representing the numbers of targets */
int targ_count = 0;

/* Global variable indicating the default block_size ls must take */
int block_size = DEFAULT_BLK_SIZE;

char * TARGET_PATH;

int 
ls_main(int argc, char * argv[])
{

 	/* 
	 * Prevent from reallocating options structure if on a recrusion
	 * usr_opt wil be alloced only once, on the first call of ls_main
	 */
	if (usr_opt == NULL) {
		usr_opt = calloc(1, sizeof(UsrOptions));
		if (!usr_opt) {
			throw_error(NULL, MEM_ERR);
			return errno;
		}

		/* Set default options for non printable characters and root */
		RootOptionSet(usr_opt);
		NonPrintableOptionSet(usr_opt);
	}

	/* 
	 * Create two list that will contains the file targets 
	 * and the directory targets, then tokenize the input
	 * to fill thoses lists to use for the listing.
	 */
	TargList * file_list = calloc(1, sizeof(TargList));
	TargList * dir_list = calloc(1, sizeof(TargList));

	if (file_list == NULL || dir_list == NULL) {
	    if (file_list) {
		    TargLfree(file_list);
	    }
	    if (dir_list) {
		    TargLfree(dir_list);
	    }
	    throw_error(NULL, MEM_ERR);
	    return errno;
	}
	
	tokenize(argc, argv, file_list, dir_list);

	/* 
	 * Suppress "the not a file or directory" error 
	 * as we might have encountered a valid target 
	 */ 
	if (errno && errno != ENOENT) {
		free(usr_opt);
		TargLfree(file_list);
		TargLfree(dir_list);
		return errno;
	}

	/* 
	 * If no error has been encountered during the tokenize phase
	 * But we did not have any targets (file or directory)
	 * then add the default target './' depending on the -d option
	 */
	if (targ_found == 0)
	{
	    TargList * target_list;
	    if (usr_opt->d) {
		target_list = file_list;
	    }
	    else {
		target_list = dir_list;
	    }

	    /* 0 to mark and insert "." as a non-hidden target */
	    TargLinsert(target_list, ".", 0);
	}
	
	/* Get environement values and check for validity */
	if (usr_opt->s) {
	    block_size = GetBlockSize();
	}
	
	if (FilesProcess(file_list->next)) {
	    TargLfree(file_list);
	    TargLfree(dir_list);
	    return errno;
	}

	if (DirectoriesProcess(dir_list->next)) {
	    TargLfree(file_list);
	    TargLfree(dir_list);
	    return errno;
	}
	
	TargLfree(file_list);
	TargLfree(dir_list);

	return errno;
}


/* Wrapper function of ls, that allows option preserving
 * in between calls of ls_main. that way, the option structure is freed only when exiting in the wrapper*/
int 
wrapper_ls(int argc, char ** argv)
{

	/* First call that will setup Options and Targets */
	PINFOS = calloc(sizeof(PrintInfos), 1);

       	ls_main(argc, argv);

	if (usr_opt != NULL) {	
	    free(usr_opt);
	}

	if (PINFOS != NULL) {
	    free(PINFOS);
	}
	
	if (TARGET_PATH != NULL) {
	    free(TARGET_PATH);
	}
	
	return errno;
}

