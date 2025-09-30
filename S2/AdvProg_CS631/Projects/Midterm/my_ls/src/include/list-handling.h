#ifndef LIST_HANDLING_H
#define LIST_HANDLING_H

#include <sys/stat.h>

/* Structure that will contain the files to print for a given target*/

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


typedef struct FileList {
    char * fname;
    int ishidden: 1;

    struct stat sb;
    struct FileList * next;
} FileList;


void ResetPrintInfos(PrintInfos * infos);

void 
FileListFree(FileList * list);

int 
FileListInsert(char * dir, char * filename, 
		FileList * filelist, FileList * reclist);

void 
FileListLog(FileList * list);


#endif /* ! LIST_HANDLING_H*/
