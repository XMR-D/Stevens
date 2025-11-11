#pragma once /* CMD_PARSER_H */

#include "opt-parser.h"

typedef struct Pipeline {
        char **cmd;
        char *in_redir_target;
        char *out_redir_target;
        int append;
        int nb_tokens;
        struct Pipeline *next;
} Pipeline;

void free_pipeline(Pipeline *pipeline);
void log_pipeline(Pipeline *pipeline);
Pipeline *cmd_parser(char *input, int *nb_commands);

/* !CMD_PARSER_H */
