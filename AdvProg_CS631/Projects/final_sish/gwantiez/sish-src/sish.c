#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <errno.h>

#include "cmd-parser.h"
#include "opt-parser.h"
#include "pipeline-exec.h"
#include "shell.h"

/*
 * sish_main: main sish routine, check on arguments, parse the options
 * start up a shell or exec a single command depending on the options
 * given
 *
 * Note : return the last command executed status code, or EXIT_FAILURE
 * 	  in case a parsing error as been encountered.
 *
 * 	  comply with the man page requirement. 0 if everything went good
 * 	  >0 if an error has been encountered.
 */
static int 
sish_main(int argc, char *argv[], UsrOptions * usr_opt)
{
	int retcode = EXIT_SUCCESS;

	retcode = parse_options(&argc, &argv, usr_opt);

	/* Based on Options passed by the user, start a shell or not*/
	if (!usr_opt->c) {
		if (argc > 0) {
			errx(1, "invalid argument\nsish [-x] [-c command]");	
		}
		retcode = shell(usr_opt);
	}
	else {
		if (argc == 0 || argc > 1) {
			errx(1, "invalid argument\nsish [-x] [-c command]");	
		}
		
		int nb_commands = 0;
		int cmd_len = strlen(argv[0]);

		/* 
		 * Place a '\n' at the end of the string to prepare
		 * it for parsing
		 */
		argv[0][cmd_len] = '\n';

		Pipeline * p = cmd_parser(argv[0], &nb_commands);

		/* Parsing error encounter */
		if (p == NULL) {
			return EXIT_FAILURE;
		}

		retcode = exec_pipeline(p, nb_commands);
		free_pipeline(p);

	}	
	return retcode;
}

/*
 * main: entry point of the program, set the SHELL env variable
 * calloc the options, and call sish.
 *
 * Note: 
 * 	return the last command executed status code by sish
 * 	whenever -c is set or not
 */
int 
main(int argc, char *argv[]) 
{
	int retcode = EXIT_SUCCESS;

	/* Set SHELL environment variable */
	char * exe_path = realpath("/proc/self/exe", NULL);
	if (exe_path) {
    		setenv("SHELL", exe_path, 1);
    		free(exe_path); 
	}

	UsrOptions * usr_opt = calloc(1, sizeof(UsrOptions));
	if (usr_opt == NULL) {
		errx(1, "memory error: %s\n", strerror(errno));
	}

	retcode = sish_main(argc, argv, usr_opt);

	free(usr_opt);

	return retcode;
}
