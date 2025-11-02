#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmd-parser.h"
#include "signals-handling.h"

char ** cmd;
int nb_tokens;

//TODO: REMOVE WHEN THE PARSER WILL BE FINISHED
static void
log_cmd(char ** command, int nb_tokens) 
{
	int i = 0;
	if (command == NULL) {
		printf("(no command)\n");
	}
	while (command[i] != NULL) {
		printf("\"%s\"\n ", command[i]);
		i++;
	}
	printf("\nnb_tokens : %d\n", nb_tokens);
}

static void
log_pipeline(Pipeline * pipeline) 
{
	Pipeline * curr = pipeline;
	while (curr != NULL) {
		printf("Command:\n");
		log_cmd(curr->cmd, curr->nb_tokens);
		curr = curr->next;
	}
}


/*
 * push_in_cmd: Routine that push a string into the cmd
 * the cmd is the array that hold all the tokens
 *
 * Note: the cmd always contains one NULL string 
 * as a terminating NULL string, to conserve it, 
 * new tokens are pushed at nb_token-2 (the penultimate spot) 
 * and a NULL is pushed at nb_token-1 (the last spot)
 */ 
static int
push_in_cmd(char * token)
{
	int dup_len = strlen(token) + 1;
	char * tok_dup = calloc(dup_len, sizeof(char));
	if (tok_dup == NULL) {
		warnx("sish: parsing error: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	strcpy(tok_dup, token);
	
	char ** new_cmd = 
		realloc(cmd, ((nb_tokens + 1) * sizeof(char*)));

	if (new_cmd == NULL) {
		free(tok_dup);
		warnx("sish: parsing error: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	new_cmd[nb_tokens - 1] = tok_dup; 
	new_cmd[nb_tokens] = NULL;	
	cmd = new_cmd;
	nb_tokens++;
	return EXIT_SUCCESS;
}

static int
push_in_pipeline(Pipeline ** pipeline, char ** cmd) 
{

	Pipeline * new_elm = calloc(1, sizeof(Pipeline));
	if (new_elm == NULL) {
		warnx("sish: parsing error: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	new_elm->cmd = cmd;
	new_elm->nb_tokens = nb_tokens;
	new_elm->next = NULL;

	//If the given pipeline is NULL, set new_elm as the pipeline
	if (*(pipeline) == NULL) {
		*(pipeline) = new_elm;
		return EXIT_SUCCESS;
	}

	Pipeline * curr = *pipeline;
	while (curr->next != NULL) {
		curr = curr->next;
	}

	curr->next = new_elm;
	return EXIT_SUCCESS;
}
	
static void
free_cmd(char ** cmd)
{
	char ** curr = cmd;
	while (*curr != NULL) {
		free(*curr);
		curr++;
	}
	free(cmd);
}

static void
free_pipeline(Pipeline * pipeline)
{
    Pipeline * curr = pipeline;
    while (curr != NULL) {
        Pipeline * next = curr->next;
	free_cmd(curr->cmd);
        free(curr);
        curr = next;
    }
}

static char *
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
			else if (*curr_char == '\0'
					|| *curr_char == '\n'
						|| *curr_char == '|') {
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
			if (*curr_char != ' ' 
					&& *curr_char != '\n' 
						&& *curr_char != '|') {
				curr_char++;
				next_state = IN_TOKEN;
				break;
			}

			/* 
			 * Else we reached either the end or a delim
			 * in this case, push the token into
			 * the token array and go to the appropriate state
			 */
			saved = *curr_char;
			*curr_char = '\0';

			if (push_in_cmd(curr_tok)) {
				return NULL;
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
			if (*curr_char == '\n') {
				*curr_char = '\0';
			} else {
				curr_char++;
			}	
			return curr_char;
		default:
			/* NEVER REACHED */
			break;
	}
	return parse_machine(curr_char, curr_tok, next_state);
}

int 
cmd_parser(char * input)
{
	int last_status = 0;
	Pipeline * pipeline = NULL;
	char * in = NULL;
	char * free_ptr = NULL;
	
	in = strdup(input);
	free_ptr = in;

	while (*in != '\0') {
		
		cmd = calloc(1, sizeof(char *));
		cmd[0] = NULL;
		nb_tokens = 1;

		in = parse_machine(in, in, DELIM);
		printf("in: %s\n", in);
		
		if (in == NULL) {
			free(free_ptr);
			free_pipeline(pipeline);
			return EXIT_FAILURE;	
		}

		push_in_pipeline(&pipeline, cmd);
		cmd = NULL;

	}

	free(free_ptr);
	printf("Pipeline after parsing :\n");
	log_pipeline(pipeline);

	/* TODO: Change it for a proper command_handler that handle exit*/
	if  (((pipeline->cmd[0]) != NULL) && strcmp((pipeline->cmd)[0], "exit") == 0) {
		free_pipeline(pipeline);
		restore_term_suspend_signals();
		exit(last_status);
	}

	free_pipeline(pipeline);


	return EXIT_SUCCESS;
}
