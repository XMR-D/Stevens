#pragma once /* CMD_PARSER_H */

#include "opt-parser.h"

/* 
 * Parsing State machine enumeration used for input parsing
 *
 * DELIM State : initial state, 
 * trigger : if the current char handled is a ' ', a '\n' or a '|'
 * 
 * effect : in this state skip the next char until a IN_REDIRECTION
 * or a IN_TOKEN state is triggered.
 *
 *
 * IN_TOKEN State :
 * trigger : if the current char is different from any control character 
 * or a redirection character.
 *
 * effect: when in this state, curr_tok is no longer updated while the 
 * IN_TOKEN state is in effect. Upon reaching a delim char, a new token
 * is created and put in the actual command table then we reenter the
 * DELIM state if the delim char is not '\n' or '|' otherwise the END
 * state is called.
 *
 *
 * IN_REDIRECTION State :
 * trigger : if the char handled is '<' or '>' and the last state
 * is different than IN_REDIRECTION.
 * if last state is IN_REDIRECTION an invalid state occurs if : 
 * 	'>' or '<' is followed by '<', '>', '|', '\n' and no token have
 * 	been found upon reaching them.
 *
 * 	in this case, if in shell mode, the parsing stop an error
 * 	is printed on stderr and the user can type a new command
 *
 * 	if in command mode, an error is printed on stderr and sish
 * 	exit
 *
 * effect : skip the chars until a delim is found while checking
 * for invalid states, upon finding a delim, and depending on the
 * nature of the triggering redirection char,
 * store the token found as the input redirection target ('<'), 
 * an output redirection target  ('>') or an  output append target
 * (">>");
 *
 *
 * END STATE:
 * trigger :  if the char handled is '\n' or '|'
 *
 * effect : if the char is '\n', place a '\0' in the string
 * to null terminate it and signal the parser to stop here.
 * if the char is '|', increment the current_char to the next
 * character, and signal the parser that we are exiting on a 
 * pipe, so that it can handle invalid pipe state
 * 
 */
typedef enum ParseState {
	DELIM = 0,
	IN_TOKEN = 1,
	IN_REDIRECTION = 2,
	END = 3,
} ParseState;

typedef struct Pipeline {
	char ** cmd;
	char * in_redir_target;
	char * out_redir_target;
	int append;
	int nb_tokens;
	struct Pipeline * next;
} Pipeline;

void free_pipeline(Pipeline * pipeline);
void log_pipeline(Pipeline * pipeline); 
Pipeline * cmd_parser(char * input, int * nb_commands);

/* !CMD_PARSER_H */
