#include "portability.h"

#include <err.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "cmd-parser.h"
#include "logging.h"
#include "opt-parser.h"
#include "pipeline-exec.h"
#include "signals-handling.h"

#include "shell.h"

/* Shell signal used to indicate to procees with the fork-exec process */
#define SHELL_SIG_PROCEED 0

/*
 * Shell signal used to indicate that the next action to take is to execute
 * the next command. ignoring the fork-exec process.
 */
#define SHELL_SIG_CONTINUE 1

/*
 * Shell signal used to indicate that the next action to take is to stop
 * the shell.
 */
#define SHELL_SIG_BREAK 2

extern int put_in_background;
int last_back_pid = -1;

static void
background_process_handler(Pipeline *p, int nb_commands)
{
        int back_pid = fork();

	/* 
	 * here we cannot use the fork() call result directly
	 * it's necessary to avoid potential race conditions 
	 */
	last_back_pid = back_pid;

        /* background process */
        if (back_pid == 0) {

                /*
                 * detach the background process from
                 * this terminal
                 */
                setsid();
                exec_pipeline(p, nb_commands, 1);
                /*
                 * It would not make sense to try to
                 * track asynchronous exit status as it
                 * could create race conditions on the
                 * status error code. so in any case
                 * exit with 0, this return code will
                 * not be used anyway
                 */
		last_back_pid = 0;
                exit(0);
        } else {
                /* reap the children */
                signal(SIGCHLD, SIG_IGN);
                /* still in parent (the shell) */
                printf("[%d]\n", back_pid);
        }
}

/*
 * handle_builtins : routine to execute the builtins passed by
 * the user
 *
 * Note:
 * the routine handle cd and exit as special builtins,
 *
 * if it's the only command passed to the terminal, handle it before forking.
 * if passed within multiple command pipeline (nb_command > 1)
 * then the builtin will be ignored here,
 * and executed into a child process resulting of changing the child environment
 * showing no difference for the user who will be in the shell process
 *
 * This is Netbsd shell behavior
 */
static int
handle_builtins(Pipeline *p, int nb_commands)
{
        if ((strcmp(p->cmd[0], "exit") == 0) && nb_commands == 1) {
                return SHELL_SIG_BREAK;
        }

        if ((strcmp(p->cmd[0], "cd") == 0) && nb_commands == 1) {
                cd_main(p->nb_tokens - 1, p->cmd);
                return SHELL_SIG_CONTINUE;
        }

        return SHELL_SIG_PROCEED;
}

/*
 * read_terminal : Routine that will read the terminal
 * handling quote characters and re-reading if a unclosed quote
 * statement is found.
 *
 * Note : None
 *
 */
static char *
read_terminal(void)
{
        char *string_read = NULL;
        size_t size = 0;

        if (getline(&string_read, &size, stdin) == -1) {
                if (string_read) {
                        free(string_read);
                }
                errx(1, "error: %s\n", strerror(errno));
        }
        return string_read;
}

/*
 * shell : Core Routine of shell.c that will setup a shell
 * read the input parse it until user chose to exit it.
 *
 * Silence SIGINT, SIGQUIT and SIGTSTP signals to be sure the shell
 * cannot be interrupted.
 *
 */
int
shell(UsrOptions *usr_opt)
{
        char *input_cmd;
        int nb_commands = 0;
        int last_status = 0;
        int shell_sig = 0;

        ignore_term_suspend_signals();

        /*
         * Infinite loop which is the body of the shell,
         * breaked when the user specifically asked for the exit builtin
         * or any memory errors has been encountered. In the second
         * case, the cmd_parser break the while, every ressources
         * are freed and we return -1 to be sure that no other exit status can
         * conflict with it, and we keep a clear output for the user.
         */
        while (1) {
                printf("sish$ ");
                input_cmd = read_terminal();

                if (strcmp(input_cmd, "\n") == 0) {
                        nb_commands = 0;
                        free(input_cmd);
                        continue;
                }

                Pipeline *p = cmd_parser(input_cmd, &nb_commands);

                if (p == NULL) {
                        nb_commands = 0;
                        free(input_cmd);
                        continue;
                }

                /* If logging is needed call logging.c */
                if (usr_opt->x) {
                        log_pipeline(p);
                }


                /*
                 * handle execution taking into account
                 * backgrounding and builtins
                 */
                shell_sig = handle_builtins(p, nb_commands);

                if (shell_sig == SHELL_SIG_BREAK) {
                        free_pipeline(p);
                        break;
                } else if (shell_sig != SHELL_SIG_CONTINUE) {
                        if (put_in_background) {
                                background_process_handler(p, nb_commands);
                        } else {
                                last_status = exec_pipeline(p, nb_commands, 0);
                        }
                }
                put_in_background = 0;
                free_pipeline(p);
                free(input_cmd);
                nb_commands = 0;
        }

        free(input_cmd);

        /* restore the signals if we are exiting */
        restore_term_suspend_signals();
        return last_status;
}
