#ifndef LISTING_H
#define LISTING_H

#include <sys/stat.h>

/* Create a structure that will handle files found by the listing,
   Put files in the list depending on the metric option (-t -u -S) 
   and the sort unsort option (-f)
   Revert the list if reverse order (-r) is needed
   Once the list finished, go to the long format printer (-l) if needed
   Or the casual file printer if long format printer not specify.

   BEFORE GOING TO ANY PRINTER TAKE INTO ACCOUNT THE ENV VARIABLE 
   OR THE SIZE FORMAT OPTIONS IF SPECIFIED

   BEFORE GOING TO PRINT, RECURSIVELY GO AND CALL MY_LS ON EACH DIR AND SYMLINKS

   All options are in usr_opt

   All targets are in targlist

   The structure here should be lightweight 

FileListAppend(int metric, int order, .....) */

typedef struct FileList {
    struct stat * st;
    struct FileList * next;
    struct FileList * prev;
} FileList;

FileListAppend(FileList * head);

#endif /* !LISTING_H */