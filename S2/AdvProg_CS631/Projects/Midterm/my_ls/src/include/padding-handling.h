#ifndef PADDING_HANDLING_H
#define PADDING_HANDLING_H

#include <sys/stat.h>

#include "opt_parser.h"

/* Structure that will contain the files to print for a given target */

typedef struct PaddingInfos {

    int max_link_nb_len;
    int max_uid_len;
    int max_gid_len;
    int max_uid_int_len;
    int max_gid_int_len;
    int max_nb_byte_len;
    int max_nb_block_len;

    int max_inode_nb_len;

    long double total_bytes;
    long double total_blocks;
} PaddingInfos;


void ResetPrintInfos(PaddingInfos * infos);

void ComputePaddingNeeded(struct stat sb, UsrOptions * usr_opt);


#endif /* ! PADDING_HANDLING_H */
