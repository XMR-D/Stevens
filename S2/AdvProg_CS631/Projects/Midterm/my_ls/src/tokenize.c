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
int rec_level = 0;

extern int targ_found;
extern UsrOptions * usr_opt;

int 
process_inputs(char * token, TargList * file_list, TargList * dir_list)
{

    struct stat sb;
    int ishidden;
    TargList * target_list;

    if (!strcmp(token, "--")) {
        opt_delim++;
        return 0;
    }

    if (token[0] == '-' && !opt_delim)
        return 0;

    targ_found++;

    if (fstatat(AT_FDCWD, token, &sb, symredirection) == -1) {
        throw_error(token, WRNG_TARG_ERR);
        return errno;
    }

    /* 
     * the target is a directory, hence insert in the list of directories found
     * otherwise insert in the list of files found, so that files and directories
     * are sorted separatly.
     */
    if (S_ISDIR(sb.st_mode) && !usr_opt->d) {
        target_list = dir_list;
    } else {
	target_list = file_list;
    }

    if (IsHidden(token)) {
        ishidden = 1;
    } else {
	ishidden = 0;
    }
    
    if (TargLinsert(target_list, token, ishidden)) {
        return errno;
    }
    
    return EXIT_SUCCESS;

}

int 
tokenize(int argc, char * input[], TargList * file_list, TargList * dir_list)
{
    int opt_err = 0;
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
        process_inputs(*input, file_list, dir_list);
         
	/* 
	 * If an ENOENT error has been encountered set the return code
	 * but don't stop the execution to see if the next target
	 * works. (ls behaviour on non existing target)
	 */
	if (errno != ENOENT) {
		input++;
	} else {
		return errno;
	}
    }
    return EXIT_SUCCESS;
}
