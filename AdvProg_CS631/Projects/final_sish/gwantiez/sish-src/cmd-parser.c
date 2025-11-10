#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "handle-redirections.h"
#include "signals-handling.h"

#include "cmd-parser.h"

/* Command parser globals */
char ** cmd = NULL;
int nb_tokens = 0;
int cmd_fnd = 0;

/* Redirection handling globals */
extern int append;
extern int redir_targ_found;
extern int redir_intok;
extern char *in_target;
extern char *out_target;
extern char *redir_type;

/* 
 * Signal to specify that the current pipeline
 * needs to be executed in background
 */
int put_in_background = 0;

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
	/* if the token is "" ignore it*/
	if (*token == 0) {
		return EXIT_SUCCESS;
	}

	/* else dupplicate it and store it by reallocate cmd */
	int dup_len = strlen(token) + 1;
	char * tok_dup = calloc(dup_len, sizeof(char));
	if (tok_dup == NULL) {
		warnx("parsing error: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	strcpy(tok_dup, token);
	
	char ** new_cmd = 
		realloc(cmd, ((nb_tokens + 1) * sizeof(char*)));

	if (new_cmd == NULL) {
		free(tok_dup);
		warnx("parsing error: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	new_cmd[nb_tokens - 1] = tok_dup; 
	new_cmd[nb_tokens] = NULL;	
	cmd = new_cmd;
	nb_tokens++;
	return EXIT_SUCCESS;
}

static int
push_in_pipeline(Pipeline ** pipeline, char ** cmd, 
		char * str_in, char * str_out, int append) 
{

	Pipeline * new_elm = calloc(1, sizeof(Pipeline));
	if (new_elm == NULL) {
		warnx("parsing error: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	new_elm->cmd = cmd;
	new_elm->nb_tokens = nb_tokens;

	if (str_in != NULL) {
		new_elm->in_redir_target = strdup(str_in);
	}
	if (str_out != NULL) {
		new_elm->out_redir_target = strdup(str_out);
	}

	new_elm->append = append;
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

void
free_pipeline(Pipeline * pipeline)
{
    Pipeline * curr = pipeline;
    while (curr != NULL) {
        Pipeline * next = curr->next;
	free_cmd(curr->cmd);
	free(curr->in_redir_target);
	free(curr->out_redir_target);
        free(curr);
        curr = next;
    }
}


/* return the number of shift we need to apply to skip the redirection token */
static int
is_redirection(char * str) 
{
	if (strncmp(str, ">>", 2) == 0) {
		redir_type = ">>";
		return 2;
	}

	if (*str == '>') {
		redir_type = ">";
		return 1;
	}

	if (*str == '<') {
		redir_type = "<";
		return 1;
	}
	return 0;
}

/*
 * parse_machine: Finite State Machine in charge of parsing the input
 * recalled several times as long as subcommand exist in the user prompt
 *
 * Note : For more details about how the parse machine works,
 * 	  see cmd-parser.h file, everything is detailed here
 *
 * 	  the routine return a string representing the remaining
 * 	  characters to parse, or NULL if an error has been encounter
 */
static char *
parse_machine(char * curr_char, char * curr_tok, ParseState curr_state)
{	
	char saved = 0;
	int shift = 0;
	ParseState next_state = -1;

	switch(curr_state) {
		case DELIM:
			if (*curr_char == ' ') {
				curr_tok++;
				curr_char++;
				next_state = DELIM;
				break;
			}

			if (is_delim(*curr_char)) {
    				if (!cmd_fnd) {
        				if (*curr_char == '|'
						|| *curr_char == '\n') {
            					warnx("Syntax error:"
					  "unexpected delimiter.");
        				 }
        				return NULL;
    				}
    				next_state = END;
    				break;
			}

			/* 
			 * shift indicate the number of char to skip in case we
			 * encounter a redirection char, 
			 * if shift is 0 it's not a redirection char.
			 */
			shift = is_redirection(curr_char);
			if (shift) {
				curr_char += shift;
				curr_tok = curr_char;
				next_state = IN_REDIRECTION;
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
			if (!is_delim(*curr_char)) {
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
			cmd_fnd = 1;
			if (push_in_cmd(curr_tok)) {
				return NULL;
			}
			*curr_char = saved;

			if (*curr_char == ' ') {
				curr_char++;
				curr_tok = curr_char;
				next_state = DELIM;
				break;
			}
			next_state = END;
			break;
		case IN_REDIRECTION:
			/* 
			 * check if the actual character 
			 * trigger an invalid redirection state 
			 */	
			if (is_invalid_redir_state(*curr_char)) {
				return NULL;
			}
			
			/* 
			 * if the char is a space and we are not in a redir
			 * token goto next char until we find a string 
			 */
			if (*curr_char == ' ' && !redir_intok) {
				curr_char++;
				curr_tok++;
				next_state = IN_REDIRECTION;
				break;
			}

			/* if we found a delim '|', '\n' or ' ' */
			if (is_delim(*curr_char)) {
				saved = *curr_char;
				*curr_char = '\0';
				if (update_redir_globals(curr_tok,
							redir_type)){
					return NULL;
				}
				*curr_char = saved;
				
				curr_tok = curr_char;
				redir_intok = 0;
				redir_type = NULL;
				next_state = DELIM;
				break;
			}

			redir_intok = 1;
			curr_char++;
			next_state = IN_REDIRECTION;
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


/*
 * cmd_parser: Main parsing routine, handle the calls to the parsing
 * state machine, create the pipeline linked list that will hold
 * all the necessay informations. the resulting object is meant to
 * be passed to the pipeline_exec func
 *
 * Note : If the parsing fail for any reasons, the function returns
 * NULL, otherwise it returns the newly allocated pipeline.
 *
 */
Pipeline *
cmd_parser(char * input, int * nb_commands) 
{
	Pipeline * pipeline = NULL;
	char * in = NULL;
	int in_len = 0;
	char * saved_in = NULL;

	if (strcmp(input, "\n") == 0) {
		return NULL;
	}
	
	/* duplicate the input for easy mem handling */	
	in = strdup(input);
	saved_in = in;
	in_len = strlen(in);

	/* skip potential spaces characters */
	while (*in == ' ') {
		in++;

		/* 
		 * We reached the end of the input
		 * without finding a cmd
		 * so exit
		 */
		if (*in == '\n') {
			free(saved_in);
			return NULL;
		}
	}

	/* 
	* If the command is terminated by &
	* then signal the exec to put the pipeline
	* in background.
	*/
	if (saved_in[in_len - 2] == '&') {
		put_in_background = 1;
		saved_in[in_len - 2] = '\n';
	}

	while (*in != '\0') {
		
		cmd = calloc(1, sizeof(char *));
		cmd[0] = NULL;
		nb_tokens = 1;

		/* 
		 * extract informations from a chunk of the input
		 * a chunk is a portion of the input that represent
		 * either a subcommand (command between pipelines)
		 * or a full command (command with no pipeline)
		 */
		in = parse_machine(in, in, DELIM);
		
		if (in == NULL) {
			free(cmd);
			free_redirect_globals();
			free(saved_in);
			free_pipeline(pipeline);
			put_in_background = 0;
			return NULL;	
		}

		push_in_pipeline(&pipeline, cmd, in_target, out_target, append);
		*nb_commands = *(nb_commands) + 1;
			
		/* 
		 * Reinitialize parsing environment 
		 * all the data that is freed here has been dupplicated
		 * and placed in the proper data structure
		 */
		free_redirect_globals();
		append = 0;
		cmd_fnd = 0;

	}

	free(saved_in);
	return pipeline;
}
