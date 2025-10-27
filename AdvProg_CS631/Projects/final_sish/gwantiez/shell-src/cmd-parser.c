#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include "cmd-parser.h"

/*
static int
count_tokens(char * input)
{
	
}

static char**
cmd_tokenize(char * input) 
{
	int nb_tokens = count_tokens();

}
*/
int 
cmd_parser(char * input)
{
	//TODO: tokenize.
	//	parse the input and call the cmd execution.
	//
	//	the parser structures need to be adapted to redirections.
	//	or a specific data structure for redirections and pipes
	//	tokens need to be done.
	//
	
	/* 
	 * TODO: remove the /n at the end of input 
	 * TEMP Once parser is done this line will not be
	 * needeed anymore
	 */	
	input[strlen(input)-1] = '\0';
	
	printf("input passed in parser: %s\n", input);


	/* TODO: TEMP Change it so that it's support is cleaner */
	if (strcmp(input, "exit") == 0) {
		printf("exiting....\n");
		exit(0);
	}

	return EXIT_SUCCESS;
}
