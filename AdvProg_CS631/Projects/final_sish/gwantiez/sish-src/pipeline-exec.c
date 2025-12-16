#include "portability.h"

#include <sys/stat.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "builtins.h"
#include "cmd-parser.h"
#include "signals-handling.h"

#include "pipeline-exec.h"

extern int last_back_pid;
int last_status = 0;

/*
 * handle_execution: Routine that will execute a subcommand within
 * the Pipeline object pipeline, this function is called in parallel
 * for every subcommand existing in the pipeline. pipeline here
 * represent a pipeline block rather than the start of a full pipeline.
 *
 * if cmd_bin is a builtin, execute it by calling the corresponding
 * builtin main, exiting with whatever the values they returned.
 *
 * Note:
 * 	Does not return if successfull
 * 	Return an error in case execvp failed for a reason
 */
static int
handle_execution(char *cmd_bin, Pipeline *pipeline)
{
        /* handle builtins */
        if (strcmp(cmd_bin, "echo") == 0) {
                exit(echo_main(pipeline->nb_tokens - 1, pipeline->cmd,
                               last_status, last_back_pid));
        }

        /*
         * will likely show no difference but need to
         * be handle separatly to avoid execvp
         * returning a non existant command error
         */
        if (strcmp(cmd_bin, "cd") == 0) {
                exit(cd_main(pipeline->nb_tokens - 1, pipeline->cmd));
        }

        /*
         * if exit is found on the pipeline of execution
         * just exit but it will not affect the shell
         */
        if (strcmp(cmd_bin, "exit") == 0) {
                exit(0);
        }

        /* If successful does not return */
        execvp(cmd_bin, pipeline->cmd);

        /*
         * if this code is reached
         * it means the execution failed.
         */

        /* command not found */
        if (errno == ENOENT) {
                errx(127, "%s: not found", cmd_bin);
        }

        errx(127, "error while executing command \"%s\" : %s", cmd_bin,
             strerror(errno));
}

/*
 * handle_redirections: routine that will set the global redirection file
 * descriptors depending on what is in the actual pipeline. In case of
 * failing the routine return an error and the redirection target is
 * ignored and the subcommand is not executed.
 *
 * Note: None
 */
static int
handle_redirections(Pipeline *pipeline)
{

        /* craft the flags */
        int open_flags_read = O_RDONLY;
        int open_flags_write = 0;
        int write_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
        int fd_r, fd_w;

        /*
         * If append flag is needed set it
         * in any case the file here is guaranted
         * to have been created duirng parsing
         */
        if (pipeline->append) {
                open_flags_write = O_WRONLY | O_APPEND | O_CREAT;
        } else {
                open_flags_write = O_WRONLY | O_TRUNC | O_CREAT;
        }

        /*
         * If there is an input redirection set it
         * open the descriptor dup it to the STDIN
         * and close it.
         */
        if (pipeline->in_redir_target) {
                fd_r = open(pipeline->in_redir_target, open_flags_read);
                if (fd_r == -1) {
                        warnx("could not open %s: %s",
                              pipeline->in_redir_target, strerror(errno));
                        return EXIT_FAILURE;
                }

                if (dup2(fd_r, STDIN_FILENO) == -1) {
                        warnx("could not dupplicate"
                              "file desriptor for %s: %s",
                              pipeline->in_redir_target, strerror(errno));
                        return EXIT_FAILURE;
                }
                close(fd_r);
        }

        /*
         * If there is an output redirection set it
         * open the descriptor dup it to the STDOUT
         * and close it.
         */
        if (pipeline->out_redir_target) {
                fd_w =
                 open(pipeline->out_redir_target, open_flags_write, write_mode);
                if (fd_w == -1) {
                        warnx("could not open %s: %s",
                              pipeline->out_redir_target, strerror(errno));
                        return EXIT_FAILURE;
                }

                if (dup2(fd_w, STDOUT_FILENO) == -1) {
                        warnx("could not dupplicate"
                              "file descriptor for %s: %s",
                              pipeline->out_redir_target, strerror(errno));
                        return EXIT_FAILURE;
                }

                close(fd_w);
        }
        return EXIT_SUCCESS;
}

/*
 * exec_pipeline: entry point routine of all the cmd exuction logic
 * this function take a pointer to a pipeline object, and will execute it
 * handling pipeline and I/O redirections
 *
 * Note:
 * 	This function is called within the shell logic after the command has
 * been parsed into a pipeline, it will also be called when the -c flag will be
 * passed to sish.
 *
 *	Signals are restored within each child process (to be able to terminate
 *	a command), no need to restore them after the child is finished as
 *	they will be reaped
 *
 *	Signals stays suspended in the parent. (the shell flow)
 *
 *	to allow the background commands to not block the main process
 *	"background" is used to determine if the pipeline needs to wait
 *	or not, if 1 => exec_pipeline is called in background
 *	if 0 => exec_pipeline is not called in background
 *
 */
int
exec_pipeline(Pipeline *pipeline, int nb_commands, int background)
{
        /* Table of pipes that will be populated by pipes() calls*/
        int p_fd[nb_commands - 1][2];
        /* Table of Pids that will be populated by fork() pids*/
        pid_t pids[nb_commands];

        for (int i = 0; i < nb_commands - 1; i++) {
                pipe(p_fd[i]);
        }

        for (int i = 0; i < nb_commands; i++) {

                /* For each command fork and connect the pipes */
                pids[i] = fork();

                if (pids[i] == 0) {

                        /* Child (one of the subcommand) */

                        /*
                         * If on a subcommand restore the signals
                         * to be able to stop commands.
                         */
                        restore_term_suspend_signals();

                        /*
                         * If on a command different than the first one
                         * connect the stdin of the cmd to the output of
                         * the previous command.
                         */
                        if (i > 0) {
                                dup2(p_fd[i - 1][0], STDIN_FILENO);
                        }

                        /*
                         * If on a command different than the last one
                         * connect it's output to it's associate pipe.
                         */
                        if (i < nb_commands - 1) {
                                dup2(p_fd[i][1], STDOUT_FILENO);
                        }

                        /* Close all the old fds */
                        for (int j = 0; j < nb_commands - 1; j++) {
                                close(p_fd[j][0]);
                                close(p_fd[j][1]);
                        }

                        if (handle_redirections(pipeline)) {
                                return EXIT_FAILURE;
                        }

                        char *cmd_bin = pipeline->cmd[0];
                        handle_execution(cmd_bin, pipeline);
                }

                pipeline = pipeline->next;
        }

        /* Parent */

        /*
         * close all pipes for the parent process
         * since he will not need them
         */
        for (int i = 0; i < nb_commands - 1; i++) {
                close(p_fd[i][0]);
                close(p_fd[i][1]);
        }

        /*
         * Reap all the childrens and retreive the last
         * exit status.
         */

        int child_status = 0;

        if (!background) {
                for (int i = 0; i < nb_commands; i++) {
                        waitpid(pids[i], &child_status, 0);
                        if (i == nb_commands - 1) {
                                if (WIFEXITED(child_status)) {
                                        /*
                                         * If the child exited normally just set
                                         * the last_status as the exiting
                                         * status.
                                         */
                                        last_status = WEXITSTATUS(child_status);
                                } else if (WIFSIGNALED(child_status)) {

                                        /*
                                         * If for any reason a child is signaled
                                         * to terminate the status returned is
                                         * 128 + signal value this choice follow
                                         * many shell standards (bash, zsh,
                                         * dash...).
                                         */
                                        last_status =
                                         128 + WTERMSIG(child_status);
                                } else if (WIFSTOPPED(child_status)) {
                                        /*
                                         * Again for the same reasons (shell
                                         * standards) if a child is signaled to
                                         * stopped return 128 + stop signal
                                         * value.
                                         */
                                        last_status =
                                         128 + WSTOPSIG(child_status);
                                }
                        }
                }
        }
        return last_status;
}
