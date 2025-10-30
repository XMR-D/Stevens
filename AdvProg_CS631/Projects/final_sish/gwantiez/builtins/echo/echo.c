#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "echo.h"

#define ENV_RETCODE 1

/*
 * expand_var: Routine that print the value of the evironement
 * variable named env_var.
 *
 * Note: None
 */
static int
expand_var(char * env_var)
{
	char * var = getenv(env_var);

	if (var != NULL) {
		printf("%s", var);
	}
	return EXIT_SUCCESS;
}

/*
 * expand_word: Routine called when a '$' is found,
 * it will try to expand the word specified directly after.
 *
 * if it's '$' : print the current pid
 *
 * if it's '?' : print last exit status
  *
 * if it's '\0': print '$' as a symbol
 * 
 * if it's any other char that mark the start of a word: 
 * print the env variable named after it and return -1 
 * to indicate to skip to the next echo word.
 *
 * Note: None
 */
static int 
expand_word(char * word, int last_exit_status)
{
	switch(*word) {
		case '$':
			//print current PID
			printf("%d", getpid());
			break;
		case '?':
			//print last exit status
			printf("%d", last_exit_status);
			break;
		case '\0':
			//print $ as a char
			printf("$");
			break;
		default:
			//else print word as an env variable
			expand_var(word);
			return -1;
	}
	return EXIT_SUCCESS;

}

static int
print_echo_word(char * input, int last_exit_status)
{
	while (input != NULL) {	
		if (*input == '$') {
			if (expand_word(input++, last_exit_status) == -1) {
				break;
			}
		} else {
			printf("%c", *input);
		}
		input++;
	}
	return EXIT_SUCCESS;

}

int 
echo_main(int argc, char ** argv, int last_exit_status)
{
	if (argv == NULL) {
		return EXIT_SUCCESS;
	}

	for (int i = 1; i < argc; i++) {
		print_echo_word(argv[i], last_exit_status);
	}
	printf("\n");

	return EXIT_SUCCESS;
}
