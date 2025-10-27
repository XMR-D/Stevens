#include <stdlib.h>
#include <stdio.h>

#include <string.h>

#include "cmd-parser.h"

int cmd_parser(char * input)
{
	//TODO: tokenize.
	//	parse the input and call the cmd execution.
	//
	//	the parser structures need to be adapted to redirections.
	//	or a specific data structure for redirections and pipes
	//	tokens need to be done.
	//
	
	/* remove the /n at the end of input */	
	input[strlen(input)-1] = '\0';
	
	printf("input passed in parser: %s\n", input);

	if (strcmp(input, "exit") == 0) {
		printf("exiting....\n");
		exit(0);
	}

	return EXIT_SUCCESS;
}
