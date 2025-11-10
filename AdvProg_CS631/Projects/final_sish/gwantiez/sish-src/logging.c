#include <stdio.h>

#include "cmd-parser.h"

#include "logging.h"

static void
log_cmd(char **cmd, int nb_tok) 
{

	for (int i = 0; i < nb_tok; i++) {
		/* 
		 * Just to skip the NULL string at 
		 * the end of the cmd array
		 */
		if (cmd[i]) {
			printf("%s", cmd[i]);
		}

		if (i != nb_tok - 1) {
			printf(" ");
		}
	}
}

void
log_pipeline(Pipeline * p) 
{
	Pipeline * curr = p;

	while (curr != NULL) {
		printf("+ ");
		log_cmd(curr->cmd, curr->nb_tokens);
		printf("\n");
		curr = curr->next;
	}

}
