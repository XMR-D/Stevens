#ifndef LISTING_H
#define LISTING_H

#include <sys/stat.h>

#include "targ_parser.h"

#include "list-handling.h"

int ListFile(char * dir, FileList * filelist, FileList * reclist);

typedef struct PrintInfos {

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
} PrintInfos;

void 
ResetPinfos(PrintInfos * infos);

void
ComputePaddingNeeded(FileList * elm);

int 
TargetLProcess(TargList * targets);


#endif /* !LISTING_H */
