#include "logging.h"

#include <stdio.h>

#include "cmd-parser.h"

/*
 * log_cmd: Routine that log a char** cmd on one line
 * print each token separted by a ' ', except the last one
 *
 * Note: None
 */
static void
log_cmd(char **cmd, int nb_tok)
{
        for (int i = 0; i < nb_tok; i++) {
                /*
                 * Just to skip the NULL string at
                 * the end of the cmd array
                 */
                if (cmd[i]) {
                        fprintf(stderr, "%s", cmd[i]);
                }

                if (i < nb_tok - 1) {
                        fprintf(stderr, " ");
                }
        }
}

/*
 * log_pipeline: Routine that log a pipeline object
 * each subcommand are logged independently on a newline
 * prefixed by a '+'
 *
 * Note: None
 */
void
log_pipeline(Pipeline *p)
{
        Pipeline *curr = p;

        while (curr != NULL) {
                fprintf(stderr, "+ ");
                log_cmd(curr->cmd, curr->nb_tokens);
                fprintf(stderr, "\n");
                curr = curr->next;
        }
}
