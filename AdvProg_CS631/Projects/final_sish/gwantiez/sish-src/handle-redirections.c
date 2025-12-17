#include "portability.h"

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "handle-redirections.h"
#include "var-expanding.h"

/* Redirection handling globals */
int append = 0;
int redir_intok = 0;
char *in_target = NULL;
char *out_target = NULL;
char *redir_type = NULL;

/* Shell environment vars to expand tokens */
extern int last_back_pid;
extern int last_status;

/*
 * is_delim: Determine if c is a delimiter, to signal the parser
 * to stop there
 *
 * Note :
 *
 * This tiny function exist in the sole purpose of
 * making the code in cmd_parse easier to read, since it
 * does not have real suitable places in the code, it's placed
 * here since the functions in this file will only be called within the parser
 */
int
is_delim(char c)
{
        return (c == ' ' || c == '\n' || c == '|');
}

/*
 * free_redirect_globals: freeing mechanism for all the redirection handling
 * globals above.
 *
 * Note : freeing redir_type is not necessary since it's not malloced
 */
void
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

/*
 * update_redir_globals: routine that update the redirections
 * globals depending on the type of redirection and the redirection
 * target
 *
 * Note : return EXIT_FAILURE (1) if an error has been encountered
 * 	  return EXIT_SUCCESS (0) if the globals has been successfuly
 * 	  updated.
 *
 */
int
update_redir_globals(char *redirection_targ, char *type)
{
        int free_select = 0;
        int fd = 0;
	int expanded = 0;
	char * exp_targ;

        if (strcmp(type, ">>") == 0) {
                append = 1;
        } else if (*type == '>') {
                append = 0;
        } else if (*type == '<') {
                free_select = 1;
        }

	if (redirection_targ[0] == '$') {
		exp_targ = expand_tok(redirection_targ, last_status, last_back_pid);
		if (exp_targ[0] == '\0') {
			free_redirect_globals();
			warnx("Syntax error: redirection unexpected");
                        return EXIT_FAILURE;
		}
		expanded++;
	}

        if (free_select) {
                if (in_target) {
                        free(in_target);
                }
		if (!expanded) {
                	in_target = strdup(redirection_targ);
		} else {
			in_target = exp_targ;
		}

                if (!in_target) {
                        return EXIT_FAILURE;
                }
                return EXIT_SUCCESS;
        } else {

                /*
                 * Try to create the output redirection
                 * if successful close it right away
                 * as it will be reopen later if needeed
                 * It comply with option 2.a voted in class
                 */
		if (expanded) {
			redirection_targ = exp_targ;
		}

                fd = open(redirection_targ, O_CREAT | O_WRONLY);
                if (fd != -1) {
                        close(fd);
                }
                if (out_target) {
                        free(out_target);
                }
		if (!expanded) {
                	out_target = strdup(redirection_targ);
	                if (!out_target) {
        	                return EXIT_FAILURE;
               	 	}
		} else {
			out_target = exp_targ;
		}


                return EXIT_SUCCESS;
        }
}

/*
 * is_invalid_redir_state: routine that check if the current char
 * triggers a invalid redirection state by checking the
 * redirections globals defined at the top of this C file
 *
 * Note : return EXIT_FAILURE (1) if a invalid state is identified
 * 	  return EXIT_SUCCESS (0) if no invalid state is identified
 */
int
is_invalid_redir_state(char c)
{
        /*
         * If c is a \n or a | and we did not find any token then
         * it's an invalid redirection
         */
        if ((c == '|' || c == '\n') && !redir_intok) {
                if (c == '|') {
                        warnx("Syntax error: '|' unexpected");
                } else {
                        warnx("Syntax error: newline unexpected");
                }
                return EXIT_FAILURE;
        }

        /*
         * If we are in a '>' redirection,
         * if c is a '<' and no tokens have been found
         * then it's an invalid redirection
         */
        if (strcmp(">", redir_type) == 0) {
                if (c == '<' && !redir_intok) {
                        warnx("Syntax error: redirection unexpected");
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
                        warnx("Syntax error: redirection unexpected");
                        return EXIT_FAILURE;
                }
        }

        /*
         * If we are in a ">>" redirection,
         * if c is a '<' or '>' and no tokens have been found
         * then it's an invalid redirection
         */
        if (strcmp(">>", redir_type) == 0) {
                if ((c == '>' || c == '<') && !redir_intok) {
                        warnx("Syntax error: redirection unexpected");
                        return EXIT_FAILURE;
                }
        }
        return EXIT_SUCCESS;
}
