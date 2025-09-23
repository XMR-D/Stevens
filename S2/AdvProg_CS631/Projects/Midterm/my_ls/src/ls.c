#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "listing.h"
#include "opt_parser.h"
#include "targ_parser.h"
#include "tokenize.h"
#include "utility.h"

#ifndef DEFAULT_BLK_SIZE
#define DEFAULT_BLK_SIZE 512
#endif /* !DEFAULT_BLK_SIZE */

/* Global variable representing options structure */
UsrOptions * usr_opt;

/* Global variable representing if a target as been tested */
int targ_found;

/* Global variable representing the numbers of targets */
int targ_count = 0;

/* Global variable indicating if the program is launch as root */
int root = 0;

int block_size = DEFAULT_BLK_SIZE;

int 
ls_main(int argc, char * argv[])
{

	int ret = 0;
	TargList * tl_tail;

	if (!usr_opt)
	{
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

	root = CheckRoot();
       	ret = ls_main(argc, argv);

	if (usr_opt != NULL)	
	    free(usr_opt);
	
	return ret;
}

