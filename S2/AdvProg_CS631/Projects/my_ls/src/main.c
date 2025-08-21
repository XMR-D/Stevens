#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "opt_parser.h"

#include "tokenize.h"

extern UsrOptions * usr_opt;

int main(int argc, char * argv[])
{
	//TODO: Handle case where ls is called without args
	WARN("ls launched.");
	int ret = tokenize(argc, argv);
	if (ret)
		return ret;

	SUCCESS("ls finished successfully.");
	return 0;
}
