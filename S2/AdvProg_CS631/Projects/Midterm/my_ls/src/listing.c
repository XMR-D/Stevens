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
	FTS * ftsp = NULL;
	FTSENT * entry;
	int fts_flags;

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

	while((entry = fts_read(ftsp)) != NULL) {
		switch(entry->fts_info) {
			case FTS_D:
				if (argc > 1)
					continue;
			case FTS_SLNONE:
			case FTS_SL:
			case FTS_W:
			case FTS_F:
			case FTS_DC:
			case FTS_DEFAULT:
			case FTS_DP:
				default:
			break;
		}

	}
	fts_close(ftsp);

	if (errno != 0) {
		fprintf(stderr, "ls: error: %s\n", strerror(errno));
		return errno;
	}
	return EXIT_SUCCESS;
}
