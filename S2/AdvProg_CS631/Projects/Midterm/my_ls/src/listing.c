#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <fts.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "opt_parser.h"
#include "padding-handling.h"
#include "printing.h"
#include "utility.h"

#include "listing.h"

extern UsrOptions * USR_OPT;

/* 
 * Global that notify TreeTraversal that something has been output on the terminal
 * In this case we need to add an additional '\n', 
 * initialized to 0 (Nothing has been printed)
 * Can be set to 1 (Something has been printed) in LongFormatPrinter
 */
int PRINTED = 0;

static int 
FTSCompare(const FTSENT **elm1, const FTSENT **elm2)
{
    int ret = 0;
    struct stat * sb1;
    struct stat * sb2;

    sb1 = (*elm1)->fts_statp;
    sb2 = (*elm2)->fts_statp;
    
    if (USR_OPT->S) {
        ret = CompareMetrics(sb1->st_size, sb2->st_size);
    }
    else if (USR_OPT->t && USR_OPT->c) {
        ret = CompareTimeMetrics(sb1->st_ctim, sb2->st_ctim);
    }
    else if (USR_OPT->t && USR_OPT->u) {
        ret = CompareTimeMetrics(sb1->st_atim, sb2->st_atim);
    }
    else if (USR_OPT->t) {
        ret = CompareTimeMetrics(sb1->st_mtim, sb2->st_mtim);
    }
    if (ret == 0) {
        ret = strcmp((*elm1)->fts_name, (*elm2)->fts_name);
    }

    if (USR_OPT->r) {
        return ret * -1;
    }
    else {
	return ret;
    }
}

int
TreeTraversal(int argc, char * argv[])
{

	int fts_flags;
	FTS * ftsp = NULL;
	FTSENT * entry;
	FTSENT * children_dir;

	fts_flags = FTS_PHYSICAL;
	
	if (USR_OPT->a || !USR_OPT->A) {
		fts_flags |= FTS_SEEDOT;
	}

	ftsp = fts_open(argv, fts_flags, FTSCompare);

	if (ftsp == NULL) {
		fprintf(stderr, "ls: error: %s\n", strerror(errno));
		return errno;
	}

	/* Call the printer on the first batch of arguments  */
	if(LongFormatPrinter(NULL, fts_children(ftsp, 0))) {
		fts_close(ftsp);
		return errno;
	}

	/* 
	 * if we need to list directories as plain files
	 * we can return as going through them is not
	 * necessary
	 */
	if (USR_OPT->d) {
		fts_close(ftsp);
		return EXIT_SUCCESS;
	}

	while((entry = fts_read(ftsp)) != NULL) {
		switch(entry->fts_info) {
			case FTS_D:
				/* 
				 * If something has been printed add a \n to get a clear
				 * output
				 */
				if (PRINTED) {
					printf("\n%s:\n", entry->fts_path);
				}
				else if (argc > 1) {
					printf("%s:\n", entry->fts_path);
				}

				if (USR_OPT->s) {
					PrintTotalBytes();
				}

				/* 
				 * Get all children files from the actual dir
				 * and print them all
				 */
				children_dir = fts_children(ftsp, 0);
				if(LongFormatPrinter(entry, children_dir)) {
					fts_close(ftsp);
					return errno;
				}

				/* 
				 * If we don't need recursion but
				 * we have directories to list skip them
				 */
				if (!USR_OPT->R && children_dir != NULL) {
					fts_set(ftsp, entry, FTS_SKIP);
				}
				break;

		 	case FTS_DNR:
			case FTS_ERR:
				/* In case of error, just print the error reason and try to continue */
				fprintf(stderr, "ls: %s: %s\n", entry->fts_name, strerror(entry->fts_errno));
				errno = entry->fts_errno;
				break;
			case FTS_DC:
				/* In case a cycle is detected, throw an error and return to avoid side effects */
				fprintf(stderr, "ls: %s: this directory is creating a circle\n", entry->fts_name);
				errno = entry->fts_errno;
				return errno;
			default:
				break;
		}

	}
	fts_close(ftsp);

	if (errno != 0) {
		return errno;
	}
	return EXIT_SUCCESS;
}
