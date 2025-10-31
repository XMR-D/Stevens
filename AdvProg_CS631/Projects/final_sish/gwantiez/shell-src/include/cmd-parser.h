#pragma once /* CMD_PARSER_H */

/* Parsing State machine enumeration */
typedef enum ParseState {
	DELIM = 0,
	IN_SQUOTE = 1,
	IN_TOKEN = 2,
	END = 3,
} ParseState;

int cmd_parser(char * input);

/* !CMD_PARSER_H */
