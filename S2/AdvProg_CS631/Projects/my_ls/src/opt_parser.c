#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "error.h"
#include "log.h"
#include "opt_parser.h"

/* usr_opt pointer that will contain the options for later purpose */
extern UsrOptions * usr_opt;

extern int help;

int OptSet(char opt)
{
    switch (opt) {
    /* Scope Option detected */
    case 'A':
        if (usr_opt->A)
            break;
        usr_opt->A++;
        break;
    case 'a':
        if (usr_opt->a)
            break;
        usr_opt->a++;
        break;
    case 'R':
        if (usr_opt->R)
            break;
        usr_opt->R++;
        break;
    case 'd':
        if (usr_opt->d)
            break;
        usr_opt->d++;
        break;
    
    /* Sorting Option detected */
    case 'f':
        if (usr_opt->f)
            break;
        usr_opt->f++;
        break;
    case 'r':
        if (usr_opt->r)
            break;
        usr_opt->r++;
        break;
    case 't':
        if (usr_opt->t)
            break;
        usr_opt->t++;
        break;
    case 'S':
        if (usr_opt->S)
            break;
        usr_opt->S++;
        break;
    case 'c':
        if (usr_opt->c)
            break;
        if (usr_opt->u)
            usr_opt->u--;
        usr_opt->c++;
        break;
    case 'u':
        if (usr_opt->u)
            break;
        if (usr_opt->c)
            usr_opt->c--;
        usr_opt->u++;
        break;
    
    /* Format Option detected */
    case 'F':
        if (usr_opt->F)
            break;
        usr_opt->F++;
        break;
    case 'l':
        if (usr_opt->l)
            break;
        if (usr_opt->n)
            usr_opt->n--;
        usr_opt->l++;
        break;
    case 'n':
        if (usr_opt->n)
            break;
        if (usr_opt->l)
            usr_opt->l--;
        usr_opt->n++;
        break;
    case 'q':
        if (usr_opt->q)
            break;
        if (usr_opt->w)
            usr_opt->w--;
        usr_opt->q++;
        break;
    case 'w':
        if (usr_opt->w)
            break;
        if (usr_opt->q)
            usr_opt->q--;
        usr_opt->w++;
        break;
    case 'i':
        if (usr_opt->i)
            break;
        usr_opt->i++;
        break;
    /* Size format Option detected*/
    case 'h':
        if (usr_opt->h)
            break;
        usr_opt->h++;
        break;
    case 'k':
        if (usr_opt->k)
            break;
        usr_opt->k++;
        break;
    case 's':
        if (usr_opt->s)
            break;
        usr_opt->s++;
        break;
    case '?':
        return WRNG_OPT_ERR;
        break;
    }
    
    return 0;
}

void OptionLog(UsrOptions * opt)
{
    printf("Option activated:\n"
           "A=%i a=%i R=%i d=%i\n"
           "f=%i r=%i t=%i S=%i c=%i u=%i\n"
           "F=%i l=%i n=%i q=%i w=%i h=%i s=%i i=%i\n",
           opt->A, opt->a, opt->R, opt->d,
           opt->f, opt->r, opt->t, opt->S, opt->c, opt->u,
           opt->F, opt->l, opt->n, opt->q, opt->w, opt->h, opt->s, opt->i);
}