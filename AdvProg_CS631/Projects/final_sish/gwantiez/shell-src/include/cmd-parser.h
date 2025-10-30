#pragma once /* CMD_PARSER_H */

int cmd_parser(char * input);
void reset_pipeline(void); 
void free_pipeline(void); 
/* Parsing State machine enumeration */
typedef enum ParseState {
	DELIM = 0,
	IN_SQUOTE = 1,
	IN_TOKEN = 2,
	END = 3,
} ParseState;

/* !CMD_PARSER_H */
