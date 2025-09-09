#ifndef TARG_PARSER_H
#define TARG_PARSER_H

#include <sys/stat.h>
#include <sys/types.h>

#include <time.h>

typedef struct TargList {
    char * target;                  /* Name of the target */
    unsigned char isdir: 1;         /* Is the target a directory ?*/
    unsigned char ishidden: 1;      /* Is the target a hidden file ?*/
    struct stat sb;                 /* Information about the target */
    
    struct TargList * next;
    struct TargList * prev;
} TargList;

/* 
 * TargLinsert will check if the file exist and if access is granted 
 * If not it will free all the structrure and exit throwing an error
 * If everything is fine it will create a new list element and add it to target list
 */
int TargLinsert(TargList * list, char * token, int isdir, int ishidden);

/* Freeing mechanism that will free a TargList object when invoked */
void TargLfree(TargList * list);

/* Target list logging function*/
void TargLlog(TargList * list);

#endif /* !_TARG_PARSER_H */