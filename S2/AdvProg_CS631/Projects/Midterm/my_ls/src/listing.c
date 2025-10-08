#include "listing.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fts.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "opt_parser.h"
#include "padding-handling.h"
#include "printing.h"
#include "utility.h"

extern UsrOptions *USR_OPT;

/*
 * Global that notify TreeTraversal that something has been output on the
 * terminal In this case we need to add an additional '\n', initialized to 0
 * (Nothing has been printed) Can be set to 1 (Something has been printed) in
 * LongFormatPrinter(...)
 */
int PRINTED = 0;

static void
SetFTSFlag(int *fts_flags)
{

    /* By default always return FTSENT struct for the symlink themselves */
    *fts_flags = FTS_PHYSICAL;

    /*
     * Otherride the FTS_PHYSICAL rule by following symlink if thoses are
     * specified in the command line
     */
    if (!USR_OPT->F && !USR_OPT->d && !USR_OPT->l) {
        *fts_flags |= FTS_COMFOLLOW;
    }
    /*
     * If -a option is specified we need to print "." and ".."
     * when encountered during the traversal
     */
    if (USR_OPT->a) {
        *fts_flags |= FTS_SEEDOT;
    }
}

static int
FTSCompare(const FTSENT **elm1, const FTSENT **elm2)
{
    int ret = 0;
    struct stat *sb1;
    struct stat *sb2;

    sb1 = (*elm1)->fts_statp;
    sb2 = (*elm2)->fts_statp;

    if (USR_OPT->S) {
        ret = CompareMetrics(sb1->st_size, sb2->st_size);
    } else if (USR_OPT->t && USR_OPT->c) {
        ret = CompareTimeMetrics(sb1->st_ctim, sb2->st_ctim);
    } else if (USR_OPT->t && USR_OPT->u) {
        ret = CompareTimeMetrics(sb1->st_atim, sb2->st_atim);
    } else if (USR_OPT->t) {
        ret = CompareTimeMetrics(sb1->st_mtim, sb2->st_mtim);
    }
    if (ret == 0) {
        ret = strcmp((*elm1)->fts_name, (*elm2)->fts_name);
    }

    if (USR_OPT->r) {
        return ret * -1;
    } else {
        return ret;
    }
}

int
TreeTraversal(int argc, char *argv[])
{

    int fts_flags;
    int retcode;
    FTS *ftsp = NULL;
    FTSENT *entry;
    FTSENT *children_dir;

    retcode = EXIT_SUCCESS;
    SetFTSFlag(&fts_flags);

    ftsp = fts_open(argv, fts_flags, USR_OPT->f ? NULL : FTSCompare);

    if (ftsp == NULL) {
        fprintf(stderr, "ls: error: %s\n", strerror(errno));
        return errno;
    }

    /*
     * Call the printer on the command line arguments and set errno
     * if an error is encountered
     */
    retcode = LongFormatPrinter(NULL, fts_children(ftsp, 0));

    /*
     * if we need to list directories as plain files
     * we can return as going through them is not
     * necessary
     */
    if (USR_OPT->d) {
        fts_close(ftsp);
        return errno;
    }

    while ((entry = fts_read(ftsp)) != NULL) {

        switch (entry->fts_info) {
        case FTS_SL:
            // printf("\nTRAVERSAL CURRENT CWD: %s\n", getcwd(NULL, 0));
            break;
        case FTS_D:
            /*
             * If -A is specified we must print the file starting with '.'
             * So if it's not the case just skip the directory
             * But only if we are not on the command lines arguments
             * otherwise we would skip all the regular files, as '.' is
             * placed by default when not specified anything in the cmd.
             */
            if (!USR_OPT->A && entry->fts_name[0] == '.' &&
                entry->fts_level != FTS_ROOTLEVEL) {
                break;
            }

            /*
             * If something has been printed add a '\n' to get a clean
             * output
             */
            if (PRINTED) {
                printf("\n%s:\n", entry->fts_path);
            } else if (argc > 1) {
                printf("%s:\n", entry->fts_path);
            }

            /*
             * Get all children files from the actual dir
             * and print them all
             */
            children_dir = fts_children(ftsp, 0);

            LongFormatPrinter(entry, children_dir);

            /*
             * If -R is NOT specified and that
             * we found directories to list, skip them
             */
            if (!USR_OPT->R && children_dir != NULL) {
                fts_set(ftsp, entry, FTS_SKIP);
            }
            break;

        case FTS_DNR:
        case FTS_ERR:
            /*
             * In case of error, just print the error reason
             * and try to continue
             */
            fprintf(stderr, "ls: %s: %s\n", entry->fts_name,
                    strerror(entry->fts_errno));
            errno = entry->fts_errno;
            break;
        case FTS_DC:
            /*
             * In case a cycle is detected, throw an error and
             * return to avoid side effects
             */
            fprintf(stderr, "ls: %s: this directory is creating a circle\n",
                    entry->fts_name);
            errno = entry->fts_errno;
            break;
        default:
            break;
        }
    }
    fts_close(ftsp);

    if (errno == 0 && retcode != 0) {
        errno = retcode;
    }
    return errno;
}
