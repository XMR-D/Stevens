#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmd-parser.h"
#include "signals-handling.h"

char ** pipeline;
int nb_tokens;

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
		warnx("sish: parsing error while handling %s: %s", 
				tok_dup, strerror(errno));
		return EXIT_FAILURE;
	}
	
	char ** new_pipeline = realloc(pipeline, ((nb_tokens + 1) * sizeof(char*)));
	if (new_pipeline == NULL) {
		free(tok_dup);
		warnx("sish: pipeline error while handling %s: %s", 
				tok_dup, strerror(errno));
		return EXIT_FAILURE;
	}
	free(pipeline);

	new_pipeline[nb_tokens - 1] = tok_dup;
	new_pipeline[nb_tokens] = NULL;
	pipeline = new_pipeline;
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
    if (pipeline != NULL) {
        for (int i = 0; i < nb_tokens - 1; i++) {
            free(pipeline[i]);
            pipeline[i] = NULL;
        }
        pipeline[nb_tokens - 1] = NULL;
    }
    nb_tokens = 1;  // Back to initial state: just NULL terminator
}

void 
free_pipeline(void)
{
	reset_pipeline();
	free(pipeline);
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
	
	int last_status = 0;


	pipeline = calloc(1, sizeof(char *));
	pipeline[0] = NULL;
	nb_tokens = 1;

	if (parse_machine(input, input, DELIM)) {
		return EXIT_FAILURE;	
	}
	
	log_pipeline();

	if (strcmp(input, "exit") == 0) {
		free_pipeline();
		restore_term_suspend_signals();
		exit(last_status);
	}

	free_pipeline();


	return EXIT_SUCCESS;
}
