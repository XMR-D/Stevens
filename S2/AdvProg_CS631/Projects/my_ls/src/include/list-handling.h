#ifndef LIST_HANDLING_H
#define LIST_HANDLING_H

/* Structure that will contain the files to print for a given target*/
typedef struct FileList {
    char * fname;
    int ishidden: 1;
    struct stat sb;
    struct FileList * next;
} FileList;

/* Structure that will contain all the new targets we need to relaunch ls on found during listing*/
typedef struct RecList {
    char * dname;
    struct RecList * next;
} RecList;

void RecListFree(RecList * list);
int RecListInsert(char * dirname, RecList * list);
void RecListLog(RecList * list);


void FileListFree(FileList * list);
int FileListInsert(char * filename, FileList * filelist, RecList * reclist);
void FileListLog(FileList * list);


#endif /* ! LIST_HANDLING_H*/