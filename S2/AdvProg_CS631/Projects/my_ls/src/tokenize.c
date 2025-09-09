#include <sys/stat.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "log.h"
#include "opt-parser.h"
#include "targ-parser.h"
#include "utility.h"

#include "tokenize.h"
/* opt_delim is put to 1 when '--' is met, meaning next tokens are targets */
int opt_delim = 0;

/* small interrupt signal to optimize in case of help query (such as --help) */
int help = 0;

/* Signal to revert the sort during insertion */
int RevSort = 1;

extern TargList* tl_tail;
extern TargList* targ_list;
extern int targ_found;
extern UsrOptions* usr_opt;

static int 
ProcessTargets(char* token, TargList* head, TargList* tail) 
{
    struct stat sb;
    int isdir = 0;
    int ishidden = 0;

    if (!strcmp(token, "--")) {
        opt_delim++;
        return 0;
    }

    if (token[0] == '-' && !opt_delim) return 0;

    targ_found++;

    /* If the file for some reason does not open throw the error BUT continue to
     * the next one*/
    if (stat(token, &sb) == -1) {
        throw_error('\0', token, WRNG_TARG_ERR);
        return 0;
    }

    /* target is a directory, hence start at the tail of the list*/
    if (S_ISDIR(sb.st_mode)) {
        head = tail;
        isdir++;
    }
    if (IsHidden(token)) {
        ishidden++;
    }
    if (TargLinsert(head, token, isdir, ishidden))
        return errno;
    else
        return 0;
}

int 
Tokenize(int argc, char* input[], TargList* head, TargList* tail) 
{
    WARN("Tokenizer start.");
    int err = 0;

    int opt;

    while ((opt = getopt(argc, input, "AacdFfhiklnqRrSstuw")) != -1) {
        err = OptSet((char)opt);
        if (err) return err;
    }

    input++;

    /* If -r is specified reverse the sort of targets */
    if (usr_opt->r) RevSort = -1;

    for (int i = 1; i < argc; i++) {
        err = ProcessTargets(*input, head, tail);

        /* In any case update the tail */
        if (tail->next != NULL) tail = tail->next;

        if (err) return err;

        input++;
    }

    SUCCESS("Tokensizer finished successfully.");
    return 0;
}