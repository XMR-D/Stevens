#include <err.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "cmd-parser.h"
#include "shell.h"

/* 
 * check_quote_open: Routine that will check in the input
 * if a quote is present and will return the new quote status
 * based on the old one (quote_open passed as argument)
 *
 * Note: None
 */
static int
check_quote_open(char * input, int quote_open)
{
	while (*input != '\0') {
		if (*input == '"') {
			if (quote_open) {
				quote_open = 0;
			} else {
				quote_open = 1;
			}
		}
		input++;
	}
	return quote_open;
}

/* 
 * read_terminal : Routine that will read the terminal
 * handling quote characters and re-reading if a unclosed quote
 * statement is found.
 *
 * Note : the routine works like the following : 
 *
 * Place the read string into the input buffer
 *
 * then check if re-reading is necessary (if a '"' has been
 * passed without a closing one) using check_quote_open.
 *
 * while re-reading is necessary, re-read and concat the readline
 * to the input buffer. once done pass it to the parser.
 */
static char*
read_terminal(void)
{
	char * string_read = NULL;
	char * input_cmd = NULL;
	char * curr;	
	size_t size = 0, full_size = 0;
	int quote_open = 0;

	if (getline(&string_read, &size, stdin) == -1) {
		if (string_read) {
			free(string_read);
		}
		errx(1, "sish: error: %s\n", strerror(errno));
	}


	curr = string_read;
	full_size = size;
	
	input_cmd = calloc(size, sizeof(char));
	if (input_cmd == NULL) {
		free(string_read);
		errx(1, "sish: error: %s\n", strerror(errno));
	}

	input_cmd = strcat(input_cmd, string_read);
	
	quote_open = check_quote_open(curr, quote_open);
	
	while (quote_open) {
		printf("> ");
		if (getline(&string_read, &size, stdin) == -1) {
			free(input_cmd);	
			if (string_read) {
				free(string_read);
			}
			
			errx(1, "sish: error: %s\n", strerror(errno));
		}
		input_cmd = realloc(input_cmd, full_size + size);
		input_cmd = strcat(input_cmd, string_read);
		curr = string_read;
		quote_open = check_quote_open(curr, quote_open);
	}

	return input_cmd;
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

	/* Ignore Ctrl+C, Ctrl+/ and Ctrl+Z */
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		errx(1, "sish: error: %s\n", strerror(errno));
	};
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
		errx(1, "sish: error: %s\n", strerror(errno));
	};
	if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
		errx(1, "sish: error: %s\n", strerror(errno));
	};

	while(1) {
		printf("sish$ ");
		input_cmd = read_terminal();
		cmd_parser(input_cmd);
		free(input_cmd);
	}
	return EXIT_SUCCESS;
}
