#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "log.h"
#include "opt_parser.h"
#include "targ_parser.h"
#include "tokenize.h"

/* All global variables definition */
UsrOptions * usr_opt;

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

	int ret = tokenize(argc, ++argv, targ_list, tl_tail);
	if (ret)
	{
		free(usr_opt);
		TargLfree(targ_list);
		ERROR("ls finished on error");
		return ret;
	}

	if (!targ_list->next)
		TargLinsert(".", 1, 0, targ_list);



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


