/*
 * Copyright (c) 2025 Guillaume Wantiez (alias XMR)
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <sys/wait.h>

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFFSIZ 4096

static int
command(const char *string, char *outbuf, int outlen, char *errbuf, int errlen)
{
	int fd, errpfd[2] = {0}, outpfd[2] = {0};
	sigset_t set;

	char *sh_path = getenv("SHELL");

	/* Check if the shell is accessible */
	if ((sh_path == NULL) || (access(sh_path, X_OK) == -1)) {
		return -1;
	}

	/* Ignore SIGINT and SIGQUIT */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	
	/* Block SIGCHLD*/
	sigemptyset(&set);
	sigaddset(&set, SIGCHLD);
	sigprocmask(SIG_BLOCK, &set, NULL);

	/* Create two pipes for stderr and stdout and dupplicate their */
	if (pipe(errpfd) == -1 || pipe(outpfd) == -1) {
		return -1;
	}	

	/* Create a new process to exec the command */
	fd = fork();
	if (fd == -1) {
		return EXIT_FAILURE;
	}
	if (fd == 0) {
		/* Child, the one that will exec the command */
		close(outpfd[0]);
		close(errpfd[0]);

		/* Connect the stdout and stderr to our pipes*/
		if (dup2(outpfd[1], STDOUT_FILENO) == -1 
				|| dup2(errpfd[1], STDERR_FILENO) == -1) {
			return -1;
		}
		/* Exec the command by invoking the shell */
		if(execlp("sh", "sh", "-c", string, NULL) == -1) {
			return -1;
		} else {
			return EXIT_SUCCESS;
		}
	} else {
		/* 
		 * Parent the one that will read from the pipes to set the
		 * buffers. 
		 */
		close(outpfd[1]);
		close(errpfd[1]);

		if (read(outpfd[0], outbuf, outlen) == -1) {
			return -1;
		}
		if (read(errpfd[0], errbuf, errlen) == -1) {
			return -1;
		}
		wait(NULL);
	}

	return EXIT_SUCCESS;
}

int
main(void) 
{
	char out[BUFSIZ] = { 0 }, err[BUFSIZ] = { 0 };
	if (command("ls -l", out, BUFSIZ-1, err, BUFSIZ-1) == -1) {
		printf("ERROR\n");
		perror("command");
		exit(EXIT_FAILURE);
	}
	out[BUFSIZ-1] = '\0';
	err[BUFSIZ-1] = '\0';
	printf("stdout:\n%s\n", out);
	printf("stderr:\n%s\n", err);
	return EXIT_SUCCESS;
}
