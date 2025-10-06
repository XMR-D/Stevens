#ifndef TARG_PARSER_H
#define TARG_PARSER_H

#include <sys/stat.h>
#include <sys/types.h>

#include <time.h>

typedef struct TargList {
    char * target;                 
    unsigned char ishidden: 1;      
    struct stat sb;                 
    struct TargList * next;
    struct TargList * prev;
} TargList;

/* 
 * TargLinsert will check if the file exist and if access is granted 
 * If not it will free all the structrure and exit throwing an error
 * If everything is fine it will create a new list element and add it to target list
 */
int TargLinsert(TargList *, char *, int);

/* Freeing mechanism that will free a TargList object when invoked */
void TargLfree(TargList *);

/* Target list logging function*/
void TargLlog(TargList *);

int TargLcompare(struct stat *, struct stat *, char*, char*);

#endif /* !_TARG_PARSER_H */
