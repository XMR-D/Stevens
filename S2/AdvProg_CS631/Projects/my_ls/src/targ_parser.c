#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h> 
#include <string.h>


#include "error.h"
#include "log.h"
#include "opt_parser.h"
#include "utility.h"

#include "targ_parser.h"

extern int targ_count;
extern UsrOptions * usr_opt;


int TargLcompare(TargList * elm1, TargList * elm2)
{
    /* No sorting is needeed */
    if (usr_opt->f)
        return 0;

    int ret = 0;
    int skip = 0;

    char * str1 = elm1->target;
    char * str2 = elm2->target;
    if (elm1->ishidden)
    {
        char * end = strrchr(str1, '/');
        if (end != NULL)
        {
            end += 1;
            str1 = end;
        }
    }
    if (elm2->ishidden)
    {
        char * end = strrchr(str2, '/');
        if (end != NULL)
        {
            end += 1;
            str2 = end;
        }
    }          

    if (usr_opt->S)
        ret = CompareMetrics(elm1->st_size, elm2->st_size);

    else if (usr_opt->t)
        ret = CompareMetrics(elm1->st_mtim, elm2->st_mtim);

    else if (usr_opt->c)
        ret = CompareMetrics(elm1->st_ctim, elm2->st_ctim);

    else if (usr_opt->u)
        ret = CompareMetrics(elm1->st_atim, elm2->st_atim);

    else
    {
        ret = strcasecmp(str1, str2);
        skip++;
    }
    
    if (!skip && ret == 0)
        ret = strcasecmp(str1, str2);

    return ret;
}


int TargLinsert(TargList * list, char * token, int isdir, int ishidden)
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
    elm->ishidden = ishidden;
    elm->next = NULL;
    elm->prev = list;

    struct stat sb;

    if (stat(token, &sb) == -1)
    {
        throw_error('\0', token, "Failed to open", WRNG_TARG_ERR);
        return WRNG_TARG_ERR;
    }
    else
    {
        elm->st_size = sb.st_size;
        elm->st_atim = sb.st_atim.tv_sec;
        elm->st_mtim = sb.st_mtim.tv_sec;
        elm->st_ctim = sb.st_ctim.tv_sec;
    }

    /* List is empty, just append the element depending on if it's a target or not */
    if ((!isdir && !list->next) || (isdir && !list->prev))
    {
        list->next = elm;
        elm->prev = list;
        targ_count++;
        return 0;
    }

    /* target is a dir so reversed lexicographical sort starting from tail then append */
    if (isdir)
    {
        elm->isdir = 1;

        /* If the list contain at least one element*/
        if (list->prev->target && !usr_opt->f)
        {
            /* Shift while the element is not placed right and that end of list is not reached */
            while(list->prev->target && (TargLcompare(elm, list->prev) <= 0)) 
            {
                if ((list->prev == NULL) || !list->prev->isdir)
                    break;
                list = list->prev;
            }
        }
        /* If on edge case (first or last elm in the list) just append the element*/
        if (!list->next)
        {
            if (TargLcompare(elm, list) <= 0)
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

        if(!usr_opt->f)
        {
            while(list->next && (TargLcompare(elm, list->next) > 0)) 
            {
                if ((list->next == NULL) || list->next->isdir)
                    break;
                list = list->next;
            }
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
    targ_count++;
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
    list = list->next;

    if (list)
    {
        while (list != NULL)
        {
            printf("target %i: %s\n", count, list->target);
            printf("===> dir : %i\n", list->isdir);
            printf("===> hidden : %i\n", list->ishidden);
            printf("===> size : %li\n", list->st_size);
            printf("===> last access ts : %li\n", list->st_atim);
            printf("===> last modification ts : %li\n", list->st_mtim);
            printf("===> last status change ts : %li\n\n", list->st_ctim);
            count++;
            list = list->next;
        }
    }
}