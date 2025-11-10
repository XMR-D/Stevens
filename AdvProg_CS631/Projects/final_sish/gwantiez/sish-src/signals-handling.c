#include <strings.h>
#include <errno.h>
#include <err.h>
#include <signal.h>

#include "signals-handling.h"

/*
 * ignore_term_suspend_signals: routine that block the interruption,
 * quitn and stop signal.
 *
 * Note : None
 */
void
ignore_term_suspend_signals(void)
{
	/* Ignore Ctrl+C, Ctrl+/ and Ctrl+Z */
	if (signal(SIGINT, SIG_IGN) == SIG_ERR) {
		errx(1, "sish: error: %s\n", strerror(errno));
	}
	if (signal(SIGQUIT, SIG_IGN) == SIG_ERR) {
		restore_term_suspend_signals();
		errx(1, "sish: error: %s\n", strerror(errno));
	}
	if (signal(SIGTSTP, SIG_IGN) == SIG_ERR) {
		restore_term_suspend_signals();
		errx(1, "sish: error: %s\n", strerror(errno));
	}
}

/*
 * restore_term_suspend_signals: routine that restore the interruption,
 * quitn and stop signal.
 *
 * Note : None
 */
void
restore_term_suspend_signals(void)
{
	/* Ignore Ctrl+C, Ctrl+/ and Ctrl+Z */
	if (signal(SIGINT, SIG_DFL) == SIG_ERR) {
		errx(1, "sish: error: %s\n", strerror(errno));
	}
	if (signal(SIGQUIT, SIG_DFL) == SIG_ERR) {
		errx(1, "sish: error: %s\n", strerror(errno));
	}
	if (signal(SIGTSTP, SIG_DFL) == SIG_ERR) {
		errx(1, "sish: error: %s\n", strerror(errno));
	}
}
