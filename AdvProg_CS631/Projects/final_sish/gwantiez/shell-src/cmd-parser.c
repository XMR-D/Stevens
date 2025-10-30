#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmd-parser.h"


extern char ** pipeline;
extern int nb_tokens;


//TODO: REMOVE WHEN THE PARSER WILL BE FINISHED
static void
log_pipeline(void) {
	char ** ptr = pipeline;
	printf("Pipeline after parsing :\n");
	while (*ptr != NULL) {
		printf("\"%s\" ", *ptr);
		ptr++;
	}
	printf("\n");
}

/*
 * push_in_pipeline: Routine that push a string into the pipeline
 * the pipeline is the array that hold all the tokens
 *
 * Note: the pipeline always contains one NULL string 
 * as a terminating NULL string, to conserve it, 
 * new tokens are pushed at nb_token-2 (the penultimate spot) 
 * and a NULL is pushed at nb_token-1 (the last spot)
 */ 
static int
push_in_pipeline(char * token)
{
	char * tok_dup = strdup(token);
	if (tok_dup == NULL) {
		warnx("sish: parsing error: %s", strerror(errno));
		return EXIT_FAILURE;
	}
	
	pipeline = realloc(pipeline, ((nb_tokens + 1) * sizeof(char*)));
	if (pipeline == NULL) {
		free(tok_dup);
		warnx("sish: parsing error: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	pipeline[nb_tokens -1] = tok_dup;
	pipeline[nb_tokens] = NULL;
	nb_tokens++;
	return EXIT_SUCCESS;
}

static int 
parse_machine(char * curr_char, char * curr_tok, ParseState curr_state)
{	
	char saved = 0;
	ParseState next_state = -1;
	switch(curr_state) {
		case DELIM:
			if (*curr_char == ' ') {
				curr_tok++;
				curr_char++;
				next_state = DELIM;
			}
			else if (*curr_char == '\0') {
				next_state = END;
			} else {
				next_state = IN_TOKEN;
			}
			break;
		case IN_TOKEN:
			/* 
			 * If the character is anything else than a space
			 * or a \n char, it's a token char so go into the
			 * token state
			 */
			if (*curr_char != ' ' && *curr_char != '\n') {
				curr_char++;
				next_state = IN_TOKEN;
				break;
			}

			/* 
			 * Else we reached either the end or a delim
			 * in this case, push the token into
			 * the pipeline and go to the appropriate state
			 */
			saved = *curr_char;
			*curr_char = '\0';

			if (push_in_pipeline(curr_tok)) {
				return EXIT_FAILURE;
			}
			*curr_char = saved;

			if (*curr_char == ' ') {
				curr_char++;
				curr_tok = curr_char;
				next_state = DELIM;
			} else {
				next_state = END;
			}
			break;
		case END:
			
			*curr_char = '\0';
			return EXIT_SUCCESS;
			break;
		default:
			/* NEVER REACHED */
			break;
	}
	return parse_machine(curr_char, curr_tok, next_state);
}

void
reset_pipeline(void) 
{
	char ** ptr = pipeline;
	while (*ptr != NULL) {
		free(*ptr);
		ptr++;
	}
	nb_tokens = 1;
}

void
free_pipeline(void) 
{
	char ** ptr = pipeline;
	while (*ptr != NULL) {
		free(*ptr);
           	ptr++;
        }
        free(pipeline); 
	pipeline = NULL;
}

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

	if (parse_machine(input, input, DELIM)) {
		return EXIT_FAILURE;	
	}
	
	printf("input passed in parser: %s\n", input);
	log_pipeline();

	if (strcmp(input, "exit") == 0) {
		printf("exiting....\n");
		free_pipeline();
		exit(0);
	}

	return EXIT_SUCCESS;
}
