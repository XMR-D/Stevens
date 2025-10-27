#include <err.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "cmd-parser.h"

#include "shell.h"


int 
shell(void)
{
	char * user_input = NULL;
	size_t size = 0;

	printf("sish $> ");

	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		errx(1, "sish: error: %s\n", strerror(errno));
	};

	while(1) {
		if (getline(&user_input, &size, stdin) == -1) {
			errx(1, "sish: error: %s\n", strerror(errno));
		};
		cmd_parser(user_input);
		

		printf("sish $> ");

	}
	return EXIT_SUCCESS;
}
