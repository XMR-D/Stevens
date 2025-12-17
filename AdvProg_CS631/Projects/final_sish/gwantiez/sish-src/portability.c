#include "portability.h"

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>

/*
 * Code taken from sws for portability
 *
 * This code define and implem setprogname only if on linux
 */

#ifdef __linux__
static const char *sws_progname = NULL;

static void 
setprogname(const char *progname) {
    const char *slash = strrchr(progname, '/');
    if (slash) {
        sws_progname = slash + 1;
    } else {
        sws_progname = progname;
    }
}

#else

#endif

void 
program_name_setter(char * bin)
{
	setprogname(bin);
}
