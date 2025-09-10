#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "listing.h"
#include "opt_parser.h"
#include "targ_parser.h"
#include "tokenize.h"

/* Global variable representing options structure */
UsrOptions * usr_opt;
/* Global variable representing if a target as been tested */
int targ_found;
/* Global variable representing the numbers of targets */
int targ_count = 0;

int my_ls(int argc, char * argv[])
{

	TargList * tl_tail;

	int ret = 0;

	if (!usr_opt)
	{
		usr_opt = calloc(1, sizeof(UsrOptions));
		if (!usr_opt) 
		{
			throw_error('\0', NULL, MEM_ERR);
			return errno;
		}
	}

	TargList * targ_list = calloc(1, sizeof(TargList));
	if (!targ_list) 
	{
		throw_error('\0', NULL, MEM_ERR);
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

	if (!targ_list->next && (targ_found == 0))
		TargLinsert(targ_list, ".", 1, 0);

	if (TargetLProcess(targ_list))
		return errno;

	free(usr_opt);
	TargLfree(targ_list);

	return ret;
}


int main(int argc, char ** argv)
{	
	/* First call that will setup Options and Targets */
	return my_ls(argc, argv);
}
