#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "handle-redirections.h"
#include "signals-handling.h"

#include "state-machine.h"

/* Command parser globals needed from cmd-parser */
extern char ** cmd;
extern int nb_tokens;
extern int cmd_fnd;

/* Redirection globals needed from cmd-parser */
extern int cmd_fnd;
extern int redir_intok;
extern char *redir_type;

/* forward declaration of parse_machine */
char * 
parse_machine(char * curr_char, char * curr_tok, ParseState curr_state);

/*
 * push_in_cmd: Routine that push a string into the cmd an array 
 * that hold all the tokens
 *
 * Note: 
 * 	The cmd always contains one NULL string  as a 
 * 	terminating NULL string, to conserve it, 
 * 	new tokens are pushed at nb_token-2 (the penultimate spot) 
 * 	and a NULL is pushed at nb_token-1 (the last spot)
 *
 * 	Return 0 on success, return 1 on failure
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

/* 
 * is_redirection: Routine that identify a redirection sequence,
 * and return the number of shifts that needs to be performed
 * on the input to skip it before entering IN_REDIRECTION state
 *
 * Note : 
 * 	Does not return magic numbers, just the length of each
 * 	char sequence that identify a redirection, done 
 * 	this way to avoid unnecessary calls to strlen.
 */
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
 * handle_delim_state: Process the DELIM state of the parser
 * 
 * Note :
 * 	Returns next parser state string or NULL on error
 */
static char *
handle_delim_state(char *curr_char, char *curr_tok)
{
	int shift = 0;
	
	if (*curr_char == ' ') {
		curr_tok++;
		curr_char++;
		return parse_machine(curr_char, curr_tok, DELIM);
	}

	if (is_delim(*curr_char)) {
		if (!cmd_fnd) {
			if (*curr_char == '|' || *curr_char == '\n') {
				warnx("Syntax error: unexpected delimiter.");
			}
			return NULL;
		}
		return parse_machine(curr_char, curr_tok, END);
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
		return parse_machine(curr_char, curr_tok, IN_REDIRECTION);
	} else {
		return parse_machine(curr_char, curr_tok, IN_TOKEN);
	}
}

/*
 * handle_token_state: Process the IN_TOKEN state of the parser
 * 
 * Note :
 * 	Returns next parser state string or NULL on error
 */
static char *
handle_token_state(char *curr_char, char *curr_tok)
{
	char saved = 0;
	
	/* 
	 * If the character is anything else than a space
	 * or a \n char, it's a token char so go into the
	 * token state
	 */
	if (!is_delim(*curr_char)) {
		curr_char++;
		return parse_machine(curr_char, curr_tok, IN_TOKEN);
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
		return parse_machine(curr_char, curr_tok, DELIM);
	}
	return parse_machine(curr_char, curr_tok, END);
}

/*
 * handle_redir_state: Process the IN_REDIRECTION state of the parser
 * 
 * Note: 
 * 	Returns next parser state string or NULL on error
 */
static char *
handle_redir_state(char *curr_char, char *curr_tok)
{
	char saved = 0;
	
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
		return parse_machine(curr_char, curr_tok, IN_REDIRECTION);
	}

	/* if we found a delim '|', '\n' or ' ' */
	if (is_delim(*curr_char)) {
		saved = *curr_char;
		*curr_char = '\0';
		if (update_redir_globals(curr_tok, redir_type)) {
			return NULL;
		}
		*curr_char = saved;
		
		curr_tok = curr_char;
		redir_intok = 0;
		redir_type = NULL;
		return parse_machine(curr_char, curr_tok, DELIM);
	}

	redir_intok = 1;
	curr_char++;
	return parse_machine(curr_char, curr_tok, IN_REDIRECTION);
}

/*
 * handle_end_state: Process the END state of the parser
 * Returns remaining string to parse
 */
static char *
handle_end_state(char *curr_char)
{
	if (*curr_char == '\n') {
		*curr_char = '\0';
	} else {
		curr_char++;
	}	
	return curr_char;
}


/*
 * parse_machine: Finite State Machine in charge of parsing the input
 * recalled several times as long as subcommand exist in the user prompt
 *
 * Note : For more details about how the parse machine works,
 * 	  see cmd-parser.h file, everything is detailed here
 *
 * 	  the routine return a string representing the remaining
 * 	  characters to parse, or NULL if an error has been encounter.
 */
char *
parse_machine(char * curr_char, char * curr_tok, ParseState curr_state)
{	
	switch(curr_state) {
		case DELIM:
			return handle_delim_state(curr_char, curr_tok);
		case IN_TOKEN:
			return handle_token_state(curr_char, curr_tok);
		case IN_REDIRECTION:
			return handle_redir_state(curr_char, curr_tok);
		case END:
			return handle_end_state(curr_char);
		default:
			/* NEVER REACHED */
			break;
	}
	return NULL;
}
