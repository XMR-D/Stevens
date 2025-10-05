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

	int ret = 0;
	TargList * tl_tail;

	/* 
	 * Prevent from reallocating options structure if on a recrusion
	 * usr_opt wil be alloced only once, on the first call of ls_main
	 */
	if (usr_opt == NULL) {
		usr_opt = calloc(1, sizeof(UsrOptions));
		if (!usr_opt) 
		{
			throw_error(NULL, MEM_ERR);
			return errno;
		}
	}
	
	TargList * targ_list = calloc(1, sizeof(TargList));
	if (!targ_list) 
	{
	    throw_error(NULL, MEM_ERR);
	    return errno;
	}
	tl_tail = targ_list;

	/* Set default options for non printable characters and root */
	RootOptionSet(usr_opt);
	NonPrintableOptionSet(usr_opt);
	
	ret = tokenize(argc, argv, targ_list, tl_tail);

	if (ret && ret != 2)
	{
		free(usr_opt);
		TargLfree(targ_list);
		return ret;
	}

	if (!targ_list->next && (targ_found == 0) && ret == 0)
	{
	    if (usr_opt->d)
	        TargLinsert(targ_list, ".", 0, 0);
	    else
		TargLinsert(targ_list, ".", 1, 0);
	}
	
	/* Get environement values and check for validity */
	if (usr_opt->s)
	    block_size = GetBlockSize();

	if (TargetLProcess(targ_list))
	{
	    TargLfree(targ_list);
	    return errno;
	}
	
	TargLfree(targ_list);

	return ret;
}


/* Wrapper function of ls, that allows option preserving
 * in between calls of ls_main. that way, the option structure is freed only when exiting in the wrapper*/
int 
wrapper_ls(int argc, char ** argv)
{

	/* First call that will setup Options and Targets */
	int ret;

	PINFOS = calloc(sizeof(PrintInfos), 1);

       	ret = ls_main(argc, argv);

	if (usr_opt != NULL) {	
	    free(usr_opt);
	}

	if (PINFOS != NULL) {
	    free(PINFOS);
	}
	
	if (TARGET_PATH != NULL) {
	    free(TARGET_PATH);
	}
	
	return ret;
}

