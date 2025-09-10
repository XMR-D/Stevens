#ifndef LIST_HANDLING_H
#define LIST_HANDLING_H

#include <sys/stat.h>

/* Structure that will contain the files to print for a given target*/
typedef struct FileList {
    char * fname;
    int ishidden: 1;

    struct stat sb;
    struct FileList * next;
} FileList;


void FileListFree(FileList * list);
int FileListInsert(char * dir, char * filename, FileList * filelist, FileList * reclist);
void FileListLog(FileList * list);


#endif /* ! LIST_HANDLING_H*/
