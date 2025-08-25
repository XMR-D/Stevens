#include <stdio.h>
#include <stdlib.h>

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
		return ret;

	//TODO : handle case where no target are specified (call append with token "." (treat it as dir))
	//TODO : see if '.' is always interpreted as a dir if not see what to do
	//TODO : fix special case where . or .. is invoked
	//TODO : fix bug when invoking : './ls . -a 0' or './ls . ls ..'

	//TODO CRITIC : DEPORT ALL THE STEP 1 LOGIC OUTSIDE MAIN, CALL ONCE STEP 1 TO SETUP INITIAL TARGETS AND OPTIONS
	//				THEN RECALL LS if 

	WARN("Targets found after tokenization :");
	TargLlog(targ_list);
	printf("\n\n\n");
	
	WARN("Options found after tokenization :");
	OptionLog(usr_opt);


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


