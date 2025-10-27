 /* OPT_PARSER_H */
#pragma once

typedef struct UsrOptions {
	unsigned char c : 1;
	unsigned char x : 1;
} __attribute__((packed)) UsrOptions;

int parse_options(int *argc, char **argv[], UsrOptions *usr_opt);

/* !OPT_PARSER */
