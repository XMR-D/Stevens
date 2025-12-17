#include "portability.h"

/*
 * Code taken from sws for portability
 *
 * This code define and implem setprogname only if on linux
 */

#ifdef __linux__
static const char *sws_progname = NULL;

static const char *
getprogname(void) {
    return sws_progname;
}

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
#include <stdlib.h>
#endif

void 
program_name_setter(char * bin)
{
	setprogname(bin);
}
