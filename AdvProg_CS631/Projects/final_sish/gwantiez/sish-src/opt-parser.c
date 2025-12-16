#include "portability.h"

#include <err.h>
#include <stdlib.h>
#include <unistd.h>

#include "opt-parser.h"

static int
option_set(char opt, UsrOptions *usr_opt)
{
        switch (opt) {
        case 'c':
                usr_opt->c++;
                if (usr_opt->x) {
                        usr_opt->x--;
                }
                break;
        case 'x':
                usr_opt->x++;
                if (usr_opt->c) {
                        usr_opt->c--;
                }
                break;
        default:
                return EXIT_FAILURE;
                break;
        }
        return EXIT_SUCCESS;
}

/*
 * parse_cmd : Routine that will parse the options passed in the
 * command line argument by the user
 *
 * Note: Once a token that is not an option is met, the tokenizing stop
 * 	 Considering each subsequent tokens as a potential command,
 * 	 even if formated like options
 */
int
parse_options(int *argc, char **argv[], UsrOptions *usr_opt)
{
        int opt_err = 0;
        int opt_found;

        while ((opt_found = getopt(*argc, *argv, "xc")) != -1) {
                /* If an option is invalid stop exec and return the error */
                opt_err = option_set((char)opt_found, usr_opt);
                if (opt_err) {
                        warnx("usage : sish sish [-x] [-c command]");
                        return EXIT_FAILURE;
                }
        }
        *argc -= optind;
        *argv += optind;

        return EXIT_SUCCESS;
}
