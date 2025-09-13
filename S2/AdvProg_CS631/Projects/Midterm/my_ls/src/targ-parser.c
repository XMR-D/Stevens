#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
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
extern int RevSort;
extern UsrOptions * usr_opt;
extern int symredirection;

static int TargLcompare(TargList * elm1, TargList * elm2, int isdir)
{
    int ret = 0;

    char * str1 = elm1->target;
    char * str2 = elm2->target;

    
    /* Handle hidden files for alphabetical sorting */
    if (elm1->ishidden)
    {
        char * end = strrchr(str1, '/');
        if (end != NULL)
        {
            /* Skip "/." */
            end += 2;
            str1 = end;
        }
    }
    if (elm2->ishidden)
    {
        char * end = strrchr(str2, '/');
        if (end != NULL)
        {
            /* Skip "/." */
            end += 2;
            str2 = end;
        }
    }

    if (isdir)
    {
        /* 
	 * Compare two list elements depending on the leftmost sorting option 
	 * The element we want to insert is a dir, we need to invert
	 * the result because the traversal is left to right
	 * that's why we multiply by -1.
	 */
        if (usr_opt->S)
            ret = CompareMetrics(elm2->sb.st_size, elm1->sb.st_size) * -1;

        else if (usr_opt->t && usr_opt->c)
            ret = CompareTimeMetrics(elm2->sb.st_ctim, elm1->sb.st_ctim) * -1;

        else if (usr_opt->t && usr_opt->u)
            ret = CompareTimeMetrics(elm2->sb.st_atim, elm1->sb.st_atim) * -1;

        else if (usr_opt->t)
            ret = CompareTimeMetrics(elm2->sb.st_mtim, elm1->sb.st_mtim) * -1;
	
	if (ret == 0)
            ret = strcmp(str1, str2);
    }
    else
    {
        /* 
	 * The element we want to insert is a file, we don't need to invert 
	 * the result as the traversal is right to left
	 */
        if (usr_opt->S)
            ret = CompareMetrics(elm1->sb.st_size, elm2->sb.st_size);

        else if (usr_opt->t && usr_opt->c)
            ret = CompareTimeMetrics(elm1->sb.st_ctim, elm2->sb.st_ctim);

        else if (usr_opt->t && usr_opt->u)
            ret = CompareTimeMetrics(elm1->sb.st_atim, elm2->sb.st_atim);
        
        else if (usr_opt->t)
            ret = CompareTimeMetrics(elm1->sb.st_mtim, elm2->sb.st_mtim);
	
	if (ret == 0)
            ret = strcmp(str1, str2);
    }
    return ret;
}

static int InsertEmptyList(TargList *list, TargList *elm, int isdir) 
{
    if ((!isdir && !list->next) || (isdir && !list->prev)) 
    {
        list->next = elm;
        elm->prev = list;
        targ_count++;
        return 0;
    }
    return 1;
}

static int InsertDirectory(TargList *list, TargList *elm) 
{
    elm->isdir = 1;

    if (usr_opt->f)
    {
        list->next = elm;
        elm->prev = list;
        return 0;
    }

    while (list->isdir && (RevSort * TargLcompare(list, elm, elm->isdir) > 0)) 
    {
        if (!list->prev->isdir) 
            break;
        list = list->prev;
    }
      
    if (!list->next) {
        if (list->isdir && (RevSort * TargLcompare(list, elm, elm->isdir) > 0))
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
    else 
    {
        if ((RevSort * TargLcompare(list, elm, elm->isdir) > 0))
        {
            list->prev->next = elm;
            elm->prev = list->prev;
            list->prev = elm;
            elm->next = list;
        }
        else
        {
            list->next->prev = elm;
            elm->next = list->next;
            list->next = elm;
            elm->prev = list;
        }

    }
    return 0;
}

static int InsertFile(TargList *list, TargList *elm) 
{
    elm->isdir = 0;

    if (usr_opt->f)
    {
        while (list->next && !list->next->isdir)
            list = list->next;
    }
    else
    {
        while (list->next && !list->next->isdir && 
			(RevSort * TargLcompare(list->next, elm, elm->isdir) <= 0)) 
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
    return 0;
}

int TargLinsert(TargList *list, char *token, int isdir, int ishidden) 
{
    struct stat sb;
    
    TargList *elm = calloc(sizeof(TargList), 1);
    if (!elm) 
    {
        TargLfree(list);
        return errno;
    }

    elm->target = token;
    elm->isdir = isdir;
    elm->ishidden = ishidden;
    elm->next = NULL;
    elm->prev = list;

    if (fstatat(AT_FDCWD, token, &sb, symredirection) == -1) 
    {
        throw_error(token, WRNG_TARG_ERR);
        return errno;
    } 
    else 
        elm->sb = sb;

    if (InsertEmptyList(list, elm, isdir) == 0) 
        return 0;

    if (isdir && !usr_opt->d) 
        InsertDirectory(list, elm);
    else
        InsertFile(list, elm);

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
            //printf("===> dir : %i\n", list->isdir);
            //printf("===> hidden : %i\n", list->ishidden);
            //printf("===> size : %li\n", list->st_size);
            //printf("===> last access ts : %li\n", list->st_atim);
            //printf("===> last modification ts : %li\n", list->st_mtim);
            //printf("===> last status change ts : %li\n\n", list->st_ctim);
            count++;
            list = list->next;
        }
    }
}
