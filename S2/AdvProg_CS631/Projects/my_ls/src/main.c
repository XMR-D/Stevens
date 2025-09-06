#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "log.h"
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

	WARN("ls launched.");
	TargList * tl_tail;

	if (!usr_opt)
	{
		usr_opt = calloc(1, sizeof(UsrOptions));
		if (!usr_opt) 
		{
			throw_error('\0', NULL, NULL, MEM_ERR);
			return MEM_ERR;
		}
	}

	TargList * targ_list = calloc(1, sizeof(TargList));
	if (!targ_list) 
	{
		throw_error('\0', NULL, NULL, MEM_ERR);
		return MEM_ERR;
	}
	tl_tail = targ_list;

	int ret = tokenize(argc, argv, targ_list, tl_tail);
	if (ret)
	{
		free(usr_opt);
		TargLfree(targ_list);
		ERROR("ls finished on error");
		return ret;
	}

	if (!targ_list->next && (targ_found == 0))
		TargLinsert(targ_list, ".", 1, 0);

	TargLlog(targ_list);
	OptionLog(usr_opt);
	printf("target number : %i\n", targ_count);
	SUCCESS("STEP 1 : TOKENIZATION FINISHED\n");
	
	WARN("freeing structures...");
	free(usr_opt);
	TargLfree(targ_list);
	SUCCESS("ls finished successfully.");
	return 0;
}


int main(int argc, char ** argv)
{	
	/* First call that will setup Options and Targets */
	return my_ls(argc, argv);
}


