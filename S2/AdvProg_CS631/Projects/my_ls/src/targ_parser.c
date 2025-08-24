#include <stdlib.h>
#include <stdio.h>
#include <string.h> 

#include "error.h"
#include "log.h"
#include "targ_parser.h"

/* Global variable representing the target list tail, needed to update tail position*/
extern TargList * tl_tail;

int TargLappend(TargList * list, char * token, int isdir)
{
    /* Create new list element with appropriate properties */
    struct TargList * elm = malloc(sizeof(TargList));
    
    if (!elm) 
    {
        TargLfree(list);
        return MEM_ERR;
    }

    elm->target = token;
    elm->isdir = isdir;
    elm->next = NULL;
    elm->prev = list;

    /* Push element where it belongs */

    /* List is empty, just append the element and update the tail*/
    if (!list->next)
    {
        list->next = elm;
        elm->prev = list;
        tl_tail = elm;
        return 0;
    }

    /* target is a dir so reversed lexicographical sort starting from tail then append */
    if (isdir)
    {
        elm->isdir = 1;
        while((strcmp(token, list->prev->target) < 0)) {
            list = list->prev;
            if (list->next->isdir)
                break;
        }

        list->next->prev = elm;
        elm->next = list->next;
        list->next = elm;
        elm->prev = list;

    }
    /* target is a file so lexicographical sort starting from head then append */
    else
    {
        elm->isdir = 0;
        while((strcmp(token, list->next->target) > 0)) {
            list = list->next;
            if (list->next->isdir)
                break;
        }

        list->next->prev = elm;
        elm->next = list->next;
        list->next = elm;
        elm->prev = list;
    }

    /* In any case update the tail */
    if (tl_tail->next != NULL)
        tl_tail = tl_tail->next;
    return 0;
}


int TargInitDefault(TargList * list)
{
    //TODO
    list->target = "";
    return 0;
}

void TargLfree(TargList * list)
{
    if (list != NULL)  {
        while (!(list->next))
        {
            free(list->target);
            list = list->next;
        }
        free(list->target);
        free(list);
    }

}

void TargLlog(TargList * list)
{
    int count = 1;

    if (list)
    {
        while (list != NULL)
        {
            printf("target %i: %s\n", count, list->target);
            printf("===> dir : %i\n\n", list->isdir);
            count++;
            list = list->next;
        }
    }
}