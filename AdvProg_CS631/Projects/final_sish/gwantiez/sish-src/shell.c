#include <err.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "cmd-parser.h"
#include "pipeline-exec.h"
#include "signals-handling.h"

#include "shell.h"
/* 
 * read_terminal : Routine that will read the terminal
 * handling quote characters and re-reading if a unclosed quote
 * statement is found.
 *
 * Note : None
 *
 */
static char*
read_terminal(void)
{
	char * string_read = NULL;
	size_t size = 0; 

	if(getline(&string_read, &size, stdin) == -1) {
		if (string_read) {
			free(string_read);
		}
		errx(1, "error: %s\n", strerror(errno));
	}
	return string_read;
}

/*
 * shell : Core Routine of shell.c that will setup a shell
 * read the input parse it until user chose to exit it.
 *
 * Silence SIGINT, SIGQUIT and SIGTSTP signals to be sure the shell
 * cannot be interrupted.
 *
 */
int 
shell(void)
{
	char * input_cmd;
	int nb_commands = 0;
	int last_status = 0;

	ignore_term_suspend_signals();

	/* 
	 * Infinite loop which is the body of the shell, 
	 * breaked when the user specifically asked for the exit builtin 
	 * or any memory errors has been encountered. In the second
	 * case, the cmd_parser break the while, every ressources
	 * are freed and we return -1 to be sure that no other exit status can
	 * conflict with it, and we keep a clear output for the user.
	 */
	while(1) {
		printf("sish$ ");	
		input_cmd = read_terminal();
			
		Pipeline * p = cmd_parser(input_cmd, &nb_commands);

		if (p == NULL) {
			restore_term_suspend_signals();	
			return EXIT_FAILURE;
		}
		
		last_status = exec_pipeline(p, nb_commands);

		free_pipeline(p);
		free(input_cmd);
		nb_commands = 0;
	}

	free(input_cmd);
	restore_term_suspend_signals();
	return last_status;
}
