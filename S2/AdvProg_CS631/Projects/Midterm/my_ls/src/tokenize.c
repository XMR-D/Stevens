#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

/* Signal to revert the sort during insertion */
int RevSort = 1;

extern TargList * tl_tail;
extern TargList * targ_list;
extern int targ_found;
extern UsrOptions * usr_opt;

int process_targets(char * token, TargList * head, TargList * tail)
{

    struct stat sb;
    int isdir = 0;
    int ishidden = 0;

    if (!strcmp(token, "--")) {
        opt_delim++;
        return 0;
    }

    if (token[0] == '-' && !opt_delim)
        return 0;

    targ_found++;

    if (stat(token, &sb) == -1)
    {
        throw_error(token, WRNG_TARG_ERR);
        return errno;
    }

    /* 
     * the target is a directory, hence start at the tail of the list
     * and do a reverse traversal, so that files and dirs are
     * sorted separatly
     */
    if (S_ISDIR(sb.st_mode))
    {
        head = tail;
        isdir++;
    }
    if (IsHidden(token))
    {
        ishidden++;
    }
    if (TargLinsert(head, token, isdir, ishidden))
        return errno;
    else 
        return 0;

}

int tokenize(int argc, char * input[], TargList * head, TargList * tail)
{
    int opt_err = 0;
    int targ_err = 0;

    int opt;

    while((opt = getopt(argc, input, "AacdFfhiklnqRrSstuw")) != -1)
    {
	/* If an option is invalid stop exec and return the error */
	opt_err = OptSet((char) opt);
        if (opt_err)
	{
	    throw_error(NULL, WRNG_OPT_ERR);
            return opt_err;
	}
    }

    input++;

    /* If -r is specified reverse the sort of targets */
    if (usr_opt->r)
        RevSort = -1;


    for (int i = 1; i < argc; i++)
    {
        targ_err = process_targets(*input, head, tail);
             
        /* In any case update the tail */
        if (tail->next != NULL)
            tail = tail->next;

	/* 
	 * If an error has been encountered set the return code
	 * but don't stop the execution to see if the next target
	 * works.
	 */
	if (targ_err)
	    targ_err = errno;

        input++;
    }
    /* return the last target error found */
    return targ_err;
}
