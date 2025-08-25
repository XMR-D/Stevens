#ifndef TARG_PARSER_H
#define TARG_PARSER_H


typedef struct TargList {
    char * target;
    unsigned char isdir: 1;
    struct TargList * next;
    struct TargList * prev;
} TargList;

/* 
 * TargAdd will check if the file exist and if access is granted 
 * If not it will free all the structrure and exit throwing an error
 * If everything is fine it will create a new list element and add it to target list
 */
int TargLappend(char * token, int isdir, TargList * list);

/* 
 * Invoked when no target is specified by the user,
 * TargInitDefault add the default target './' to a target list
 */
int TargInitDefault(TargList * list);

/* Freeing mechanism that will free a TargList object when invoked */
void TargLfree(TargList * list);

/* Target list logging function*/
void TargLlog(TargList * list);

#endif /* !_TARG_PARSER_H */