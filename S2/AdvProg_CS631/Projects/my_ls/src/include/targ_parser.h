#ifndef TARG_PARSER_H
#define TARG_PARSER_H

#include <sys/stat.h>
#include <sys/types.h>

#include <time.h>

typedef struct TargList {
    char * target;
    unsigned char isdir: 1;
    unsigned char ishidden: 1;
    off_t st_size;    /* Size of the target */
    time_t  st_atim;  /* Time of last access */
    time_t  st_mtim;  /* Time of last modification */
    time_t  st_ctim;  /* Time of last status change */
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