#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <errno.h>

#include "opt-parser.h"
#include "shell.h"

static int 
sish_main(int argc, char *argv[], UsrOptions * usr_opt)
{
	int retcode = EXIT_SUCCESS;

	printf("Welcome to sish_main, parsing input...\n");
	
	retcode = parse_options(&argc, &argv, usr_opt);

	/* TODO: Based on Options passed by the user, start a shell or not*/
	if (!usr_opt->c) {
	    	//launch a shell
		retcode = shell();
	}
	else {
		//Execute a simple command.
	}	
	return retcode;
}


int 
main(int argc, char *argv[]) 
{
	int retcode = EXIT_SUCCESS;

	UsrOptions * usr_opt = calloc(1, sizeof(UsrOptions));
	if (usr_opt == NULL) {
		errx(1, "sish: memory error: %s\n", strerror(errno));
	}

	retcode = sish_main(argc, argv, usr_opt);

	free(usr_opt);

	return retcode;
}
