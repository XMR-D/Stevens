#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "opt_parser.h"

int
OptSet(char opt, UsrOptions *usr_opt)
{
    switch (opt) {
    /* Scope Option detected */
    case 'A':
        if (usr_opt->A) {
            break;
        }
        usr_opt->A++;
        break;

    case 'a':
        if (usr_opt->a) {
            break;
        }
        usr_opt->a++;
        usr_opt->A = 0;
        break;

    case 'R':
        if (usr_opt->R) {
            break;
        }
        usr_opt->R++;
        break;

    case 'd':
        if (usr_opt->d) {
            break;
        }
        usr_opt->d++;
        break;

    /* Sorting Option detected */
    case 'f':
        if (usr_opt->f) {
            break;
        }
        usr_opt->f++;
        usr_opt->t = 0;
        usr_opt->S = 0;
        usr_opt->c = 0;
        usr_opt->u = 0;
        break;

    case 'r':
        if (usr_opt->r) {
            break;
        }
        usr_opt->r++;
        break;

    case 't':
        if (usr_opt->t) {
            break;
        }
        usr_opt->t++;
        if (usr_opt->S) {
            usr_opt->S = 0;
        }
        break;

    case 'S':
        if (usr_opt->S) {
            break;
        }
        usr_opt->S++;
        if (usr_opt->t) {
            usr_opt->t = 0;
        }
        break;

    case 'c':
        if (usr_opt->c) {
            break;
        }
        usr_opt->c++;
        usr_opt->u = 0;

        break;

    case 'u':
        if (usr_opt->u) {
            break;
        }
        usr_opt->u++;
        usr_opt->c = 0;
        break;

    /* Format Option detected */
    case 'F':
        if (usr_opt->F) {
            break;
        }
        usr_opt->F++;
        break;

    case 'l':
        if (usr_opt->l) {
            break;
        }
        if (usr_opt->n) {
            usr_opt->n--;
        }
        usr_opt->l++;
        break;

    case 'n':
        if (usr_opt->n) {
            break;
        }
        usr_opt->n++;
        break;

    case 'q':
        if (usr_opt->q) {
            break;
        }
        if (usr_opt->w) {
            usr_opt->w--;
        }
        usr_opt->q++;
        break;

    case 'w':
        if (usr_opt->w) {
            break;
        }
        if (usr_opt->q) {
            usr_opt->q--;
        }
        usr_opt->w++;
        break;

    case 'i':
        if (usr_opt->i) {
            break;
        }
        usr_opt->i++;
        break;
    /* Size format Option detected*/
    case 'h':
        if (usr_opt->h) {
            break;
        }
        usr_opt->h++;
        break;

    case 'k':
        if (usr_opt->k) {
            break;
        }
        usr_opt->k++;
        break;

    case 's':
        if (usr_opt->s) {
            break;
        }
        usr_opt->s++;
        break;

    case '?':
        return WRNG_OPT_ERR;
        break;
    }

    return 0;
}

void
RootOptionSet(UsrOptions *opt)
{
    uid_t euid = geteuid();

    if (euid == 0) {
        if (opt->A == 0) {
            opt->A++;
        }
    }
}

/*
 * Checking for whenever the output is to a terminal or a file
 * And depending on that, set the default printing behaviour
 * for non printable characters
 */
void
NonPrintableOptionSet(UsrOptions *opt)
{
    if (isatty(STDOUT_FILENO)) {
        if (!opt->w && !opt->q) {
            opt->q++;
        }
    } else {
        if (!opt->w && !opt->q) {
            opt->w++;
        }
    }
}

void
OptionLog(UsrOptions *opt)
{
    printf("Option activated:\n"
           "A=%i a=%i R=%i d=%i\n"
           "f=%i r=%i t=%i S=%i c=%i u=%i\n"
           "F=%i l=%i n=%i q=%i w=%i h=%i s=%i i=%i\n",
           opt->A, opt->a, opt->R, opt->d, opt->f, opt->r, opt->t, opt->S,
           opt->c, opt->u, opt->F, opt->l, opt->n, opt->q, opt->w, opt->h,
           opt->s, opt->i);
}
