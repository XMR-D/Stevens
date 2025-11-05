#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmd-parser.h"
#include "signals-handling.h"

char ** cmd = NULL;
int nb_tokens = 0;
int cmd_fnd = 0;

/* Redirection handling globals */
int redir_targ_found = 0;
int redir_intok = 0;
char *redir_type = NULL;
char *in_target = NULL;
char *out_target = NULL;
int append = 0;


//TODO: REMOVE WHEN THE PARSER WILL BE FINISHED
static void
log_cmd(char ** command, int nb_tokens) 
{
	int i = 0;
	if (command == NULL) {
		printf("(no command)\n");
	}
	printf(" ");
	while (command[i] != NULL) {
		printf("\"%s\"\n ", command[i]);
		i++;
	}
	printf("\nnb_tokens : %d\n", nb_tokens);
}

void
log_pipeline(Pipeline * pipeline) 
{
	Pipeline * curr = pipeline;
	while (curr != NULL) {
		printf("Command:\n");
		log_cmd(curr->cmd, curr->nb_tokens);
		printf("In redirection : %s\n", curr->in_redir_target);
		printf("Out redirection : %s\n", curr->out_redir_target);
		printf("Append flag : %i\n", curr->append);
		curr = curr->next;
		printf("\n\n");
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
	/* if the token is "" ignore it*/
	if (*token == 0) {
		return EXIT_SUCCESS;
	}

	/* else dupplicate it and store it by reallocate cmd */
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
push_in_pipeline(Pipeline ** pipeline, char ** cmd, 
		char * str_in, char * str_out, int append) 
{

	Pipeline * new_elm = calloc(1, sizeof(Pipeline));
	if (new_elm == NULL) {
		warnx("sish: parsing error: %s", strerror(errno));
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

/* no need to free redir_type */
static void
free_redirect_globals(void) 
{
	if (in_target) {
		free(in_target);
		in_target = NULL;
	}
	if (out_target) {
		free(out_target);
		out_target = NULL;
	}
	return;
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

static int
is_delim(char c) {
	return (c == ' ' || c == '\n' || c == '|');
}

static int
update_redir_globals(char * redirection_targ, char* type)
{
	int free_select = 0;

	if (strcmp(type, ">>") == 0) {
		append = 1;
	} else if (*type == '>') {
		append = 0;
	} else if (*type == '<') {
		free_select = 1;
	}

	if (free_select) {
		if (in_target) {
			free(in_target);
		}
		in_target = strdup(redirection_targ);
		
		if (!in_target) {
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	} else {
		if (out_target) {
			free(out_target);
		}
		out_target = strdup(redirection_targ);
		
		if (!out_target) {
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}

}

static int
is_invalid_redir_state(char c)
{
	/* 
	 * If c is a \n or a | and we did not find any token then 
	 * it's an invalid redirection
	 */
	if ((c == '|' || c == '\n') && !redir_intok) {
		warnx("sish: Syntax error: '%c' unexpected\n", c);
		return EXIT_FAILURE;
	}

	/*
	 * If we are in a '>' redirection,
	 * if c is a '<' and no tokens have been found
	 * then it's an invalid redirection
	 */
	if (strcmp(">", redir_type) == 0) {
		if (c == '<' && !redir_intok) {
		 	warnx("sish: Syntax error: redirection unexpected\n");
			return EXIT_FAILURE;
		}
	}

	/*
	 * If we are in a '<' redirection,
	 * if c is a '>' and no tokens have been found
	 * then it's an invalid redirection
	 */
	if (strcmp("<", redir_type) == 0) {
		if (c == '>' && !redir_intok) {
		 	warnx("sish: Syntax error: redirection unexpected\n");
			return EXIT_FAILURE;
		}
	}

	/*
	 * If we are in a ">>" redirection,
	 * if c is a '<' or '>' and no tokens have been found
	 * then it's an invalid redirection
	 */
	if (strcmp(">>", redir_type) == 0) 
	{
		if ((c == '>' || c == '<') && !redir_intok) {
		 	warnx("sish: Syntax error: redirection unexpected\n");
			return EXIT_FAILURE;
		}
	}
	return EXIT_SUCCESS;
}

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
			
			if (*curr_char == '\0'
					|| *curr_char == '\n'
						|| *curr_char == '|') {
				/* 
				 * If we did not find any cmd before a pipeline
				 * then it's an invalid prompt
				 */
				if (!cmd_fnd) {
					warnx("sish: Syntax error:"
					" unexpected delimiter.\n");
					return NULL;
				}
				next_state = END;
				break;
			}

			/* 
			 * shift indicate the number of char to skip in case we
			 * encounter a redirection char, if shift is 0 it's not a
			 * redirection char.
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
			//TODO: finish the implem
		
			/* check if the actual character trigger an invalid redirection state */	
			if (is_invalid_redir_state(*curr_char)) {
				return NULL;
			}
			
			/* if the char is a space and we are not in a redir token goto next char */
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

Pipeline *
cmd_parser(char * input)
{
	Pipeline * pipeline = NULL;
	char * in = NULL;
	char * saved_in = NULL;
	

	/* dupplicate the input for easy mem handling */	
	in = strdup(input);
	saved_in = in;

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
			return NULL;	
		}

		push_in_pipeline(&pipeline, cmd, in_target, out_target, append);
			
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
