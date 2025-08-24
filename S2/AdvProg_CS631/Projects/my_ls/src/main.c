#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "log.h"
#include "opt_parser.h"
#include "targ_parser.h"
#include "tokenize.h"

UsrOptions * usr_opt;
TargList * tl_tail;
TargList * targ_list;

int main(int argc, char * argv[])
{
	WARN("ls launched.");

	usr_opt = calloc(1, sizeof(UsrOptions));
	if (!usr_opt) 
	{
		throw_error('\0', NULL, NULL, MEM_ERR);
		return MEM_ERR;
	}

	targ_list = calloc(1, sizeof(TargList));
	if (!targ_list) 
	{
		throw_error('\0', NULL, NULL, MEM_ERR);
		return MEM_ERR;
	}

	tl_tail = targ_list;
	

	int ret = tokenize(argc, ++argv);
	if (ret)
		return ret;

	TargLlog(targ_list);
	printf("\n\n\n");
	OptionLog(usr_opt);

	WARN("Options found after tokenization");
	WARN("freeing structures...");
	free(usr_opt);
	TargLfree(targ_list);
	SUCCESS("ls finished successfully.");
	return 0;
}
