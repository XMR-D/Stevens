#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include "error.h"
#include "log.h"
#include "opt_parser.h"
#include "targ_parser.h"
#include "utility.h"

#include "tokenize.h"

/* opt_delim is put to 1 when '--' is met, meaning next tokens are targets */
int opt_delim = 0;

/* small interrupt signal to optimize in case of help query (such as --help) */
int help = 0;

extern TargList * tl_tail;
extern TargList * targ_list;
extern int targ_found;

int process_token(char * token, TargList * head, TargList * tail)
{
    /* token is an help request*/
    if (!strcmp(token, "--help") && !opt_delim)
    {
        help++;
        return 0;
    }

    /* If the token '--' then state that delim has been found */
    if (!strcmp(token, "--") && !opt_delim)
    {
        opt_delim++;
        return 0;
    }

        /* token is an option*/
    if (token[0] == '-' && !opt_delim)
    {
        if (OptSet(token))
            return WRNG_OPT_ERR;
        else
            return 0;

    }

    /* token is a target */
    else
    {
        struct stat sb;
        int isdir = 0;
        int ishidden = 0;
        int isinvalid = 0;

        targ_found++;

        /* target is a directory, hence start at the tail of the list*/
        if (stat(token, &sb) == -1)
        {
            isinvalid++;
            throw_error('\0', token, "No such file or directory", WRNG_TARG_ERR);
            return 0;
        }
        if (S_ISDIR(sb.st_mode))
        {
            head = tail;
            isdir++;
        }

        /* 
            Not . or .. and start with . so it's an hidden file 
            DEBUG THIS : (WHAT ABOUT PATHS SUCH AS ../../../TOTO/LEL ?)
            MAKE TOOL FUNCTIONS IN OTHER SOURCE FILE AND USE IT HERE
        */
        if (IsHidden(token))
        {
            ishidden++;
        }

        if (TargLinsert(token, isdir, ishidden, head))
            return WRNG_TARG_ERR;
        else 
            return 0;
    }
}

int tokenize(int argc, char * input[], TargList * head, TargList * tail)
{
    WARN("Tokenizer start.");
    int err = 0;

    for (int i = 1; i < argc && !help; i++)
    {
        err = process_token(*input, head, tail);
       
        /* In any case update the tail */
        if (tail->next != NULL)
            tail = tail->next;

        if (err)
            return err;

        if (help)
            return 0;

        input++;
    }
    SUCCESS("Tokensizer finished successfully.");
    return 0;
}