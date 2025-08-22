#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "opt_parser.h"
#include "tokenize.h"

UsrOptions * usr_opt;

int main(int argc, char * argv[])
{
	//TODO: Handle case where ls is called without args
	WARN("ls launched.");

	usr_opt = malloc(sizeof(UsrOptions));
	* (int *) usr_opt = 0;

	int ret = tokenize(argc, argv);
	if (ret)
		return ret;

	WARN("Options found after tokenization");

	OptionLog(usr_opt);



	SUCCESS("ls finished successfully.");
	WARN("freeing structures...");
	free(usr_opt);

	return 0;
}
