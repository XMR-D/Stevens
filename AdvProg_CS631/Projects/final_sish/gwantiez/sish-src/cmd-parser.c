#include "portability.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "handle-redirections.h"
#include "signals-handling.h"
#include "state-machine.h"
#include "var-expanding.h"

#include "cmd-parser.h"

/* Command parser globals */
char **cmd = NULL;
int nb_tokens = 0;
int cmd_fnd = 0;

/*  Signal to put current pipeline in background during execution */
int put_in_background = 0;

/* Redirection handling globals */
extern int append;
extern int redir_targ_found;
extern char *in_target;
extern char *out_target;
extern char *redir_type;

/* Shell environment vars defined in shell.c */
extern int last_status;
extern int last_back_pid;

/*
 * free_cmd: Freeing mechanism for a char ** object
 *
 * Note : None
 */
static void
free_cmd(char **cmd)
{
        char **curr = cmd;
        while (*curr != NULL) {
                free(*curr);
                curr++;
}
        free(cmd);
}

/*
 * free_pipeline: Freeing mechanism for the pipeline object
 *
 * Note : call free_cmd to free the char ** object
 */
void
free_pipeline(Pipeline *pipeline)
{
        Pipeline *curr = pipeline;
        while (curr != NULL) {
                Pipeline *next = curr->next;
                free_cmd(curr->cmd);
                free(curr->in_redir_target);
                free(curr->out_redir_target);
                free(curr);
                curr = next;
        }
}


static int
expand_cmd_toks(char ** cmd, int nb_tokens)
{
	char * tok;
	char * exp_tok;
	int new_tks_nb = nb_tokens;

	for (int i = 0; i < nb_tokens - 1; i++) {
		tok = cmd[i];

		if (tok[0] == '$') {
			exp_tok = expand_tok(tok, last_status, last_back_pid);
			if (exp_tok[0] == '\0') {
				new_tks_nb -= 1;
			}

			/*In any case update the token */
			free(cmd[i]);
			cmd[i] = exp_tok;
		}
	}

	return new_tks_nb;
}

int
expand_cmds(Pipeline * p, int nb_commands)
{
	Pipeline * curr = p;
	int new_tks_nb;

	for (int i = 0; i < nb_commands; i++) {

		/* expand and update the datas */
		new_tks_nb = expand_cmd_toks(curr->cmd, curr->nb_tokens);
		p->nb_tokens = new_tks_nb;

		/*
		 * In the following case the pipeline became invalid
		 * because one of the command disappeared
		 */
		if (new_tks_nb == 1) {
			if (nb_commands > 1) { 
				warnx("Syntax error: unexpected delimiter");
                        	return EXIT_FAILURE;
			}

			if (nb_commands == 1) {
				return EXIT_FAILURE;
			}
		}

		curr = curr->next;
	}
	return EXIT_SUCCESS;
}

/*
 * push_in_pipeline: Routine that push a char ** into the pipeline
 * it will create a pipeline block out of str_in, str_out and append value
 *
 * Note:
 * 	Called by cmd_parser after the parse machine successfully
 * 	parsed a chunk of the input.
 *
 * 	Return 0 on success and 1 on failure
 */
static int
push_in_pipeline(Pipeline **pipeline, char **cmd, char *str_in, char *str_out,
                 int append)
{

        Pipeline *new_elm = calloc(1, sizeof(Pipeline));
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

        /* If the given pipeline is NULL, set new_elm as the pipeline */
        if (*(pipeline) == NULL) {
                *(pipeline) = new_elm;
                return EXIT_SUCCESS;
        }

        Pipeline *curr = *pipeline;
        while (curr->next != NULL) {
                curr = curr->next;
        }

        curr->next = new_elm;
        return EXIT_SUCCESS;
}

/*
 * cmd_parser: Main parsing routine, handle the calls to the parsing
 * state machine, create the pipeline linked list that will hold
 * all the necessay informations. the resulting object is meant to
 * be passed to the pipeline_exec func
 *
 * Note : If the parsing fail for any reasons, the function returns
 * NULL, otherwise it returns the newly allocated pipeline.
 */
Pipeline *
cmd_parser(char *input, int *nb_commands)
{
        Pipeline *pipeline = NULL;
        char *in = NULL;
        int in_len = 0;
        char *saved_in = NULL;

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
