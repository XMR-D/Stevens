#include <stdlib.h>
#include <stdio.h>
#include <strings.h> 

#include "error.h"
#include "log.h"
#include "targ_parser.h"

int TargLappend(char * token, int isdir, int ishidden, TargList * list)
{
    /* Create new list element with appropriate properties */
    struct TargList * elm = malloc(sizeof(TargList));
    
    char * cpy = token;

    if (!elm) 
    {
        TargLfree(list);
        return MEM_ERR;
    }

    elm->target = token;
    elm->isdir = isdir;
    elm->ishidden = ishidden;
    elm->next = NULL;
    elm->prev = list;

    /* Push element where it belongs */

    /* List is empty, just append the element depending on if it's a target or not */
    if ((!isdir && !list->next) || (isdir && !list->prev))
    {
        list->next = elm;
        elm->prev = list;
        return 0;
    }

    /* target is a dir so reversed lexicographical sort starting from tail then append */
    if (isdir)
    {
        elm->isdir = 1;

        /* Skip first char (.) before sorting */
        if(ishidden)
            cpy = &(cpy[1]);

        /* If the list contain at least one element*/
        if (list->prev->target)
        {
            /* Shift while the element is not placed right and that end of list is not reached */
            while(list->prev->target && (strcasecmp(cpy, list->prev->target) <= 0)) 
            {
                if ((list->prev == NULL) || !list->prev->isdir)
                    break;
                list = list->prev;
            }
        }
        /* If on edge case (first or last elm in the list) just append the element*/
        if (!list->next)
        {
            if (strcasecmp(cpy, list->target) <= 0)
            {
                list->prev->next = elm;
                elm->prev = list->prev;
                list->prev = elm;
                elm->next = list;
            }
            else
            {
                list->next = elm;
                elm->prev = list;
            }
            
        }
        /* else insert it between two existing elements*/
        else 
        {
            list->prev->next = elm;
            elm->prev = list->prev;
            list->prev = elm;
            elm->next = list;
        }
    }
    /* target is a file so lexicographical sort starting from head then append */
    else
    {
        elm->isdir = 0;

        if(ishidden)
            cpy = &(cpy[1]);

        
        while(list->next && (strcasecmp(cpy, list->next->target) > 0)) 
        {
            if ((list->next == NULL) || list->next->isdir)
                break;
            list = list->next;
        }

        if (list->next)
        {
            list->next->prev = elm;
            elm->next = list->next;
            list->next = elm;
            elm->prev = list;
        }
        else 
        {
            list->next = elm;
            elm->prev = list;
        }
    }
    return 0;
}

void TargLfree(TargList * list)
{
    TargList * next = list->next;
    
    while (next != NULL)
    {
        free(list);
        list = next;
        next = list->next;
    }
    free(list);

}

void TargLlog(TargList * list)
{
    int count = 1;

    if (list)
    {
        while (list != NULL)
        {
            printf("target %i: %s\n", count, list->target);
            printf("===> dir : %i\n", list->isdir);
            printf("===> hidden : %i\n\n", list->ishidden);
            count++;
            list = list->next;
        }
    }
}