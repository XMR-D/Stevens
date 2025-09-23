#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
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

/* Signal to revert the sort during insertion */
int RevSort = 1;

/* Global variable representing the symlink redirection flag */
int symredirection = 0;

/*
 * small signal to indicate whenever the program is in a recursion
 * to indicate to ls if fetching option is necessary or not
 */
int rec_level;

extern TargList * tl_tail;
extern TargList * targ_list;
extern int targ_found;
extern UsrOptions * usr_opt;
extern int root;

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

    if (fstatat(AT_FDCWD, token, &sb, symredirection) == -1)
    {
        throw_error(token, WRNG_TARG_ERR);
        return errno;
    }

    /* 
     * the target is a directory, hence start at the tail of the list
     * and do a reverse traversal and insertion, so that files and dirs are
     * sorted separatly
     */
    if (S_ISDIR(sb.st_mode) && !usr_opt->d)
    {
        head = tail;
        isdir++;
    }
    if (IsHidden(token) && !root)
        ishidden++;
    
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

    while(rec_level == 0 && 
		    (opt = getopt(argc, input, "AacdFfhiklnqRrSstuw")) != -1)
    {
	/* If an option is invalid stop exec and return the error */
	opt_err = OptSet((char) opt);
        if (opt_err)
	{
	    throw_error(NULL, WRNG_OPT_ERR);
            return opt_err;
	}
    }

    if (usr_opt->d || usr_opt->F)
        symredirection = AT_SYMLINK_NOFOLLOW;
    else
	symredirection = 0;

    /* skip the first element which is the binary name*/
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
    return targ_err;
}
