#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "error.h"
#include "listing.h"
#include "opt_parser.h"
#include "utility.h"

#include "list-handling.h"

extern int RevSort;
extern UsrOptions * usr_opt;

static int FileListCompare(FileList * elm1, FileList * elm2)
{
    int ret = 0;

    char * str1 = elm1->fname;
    char * str2 = elm2->fname;

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

    /* The element is a file and need to be compared, to keep list ordering invert the result */
    if (usr_opt->S)
        ret = CompareMetrics(elm1->sb.st_size, elm2->sb.st_size) * -1;
    else if (usr_opt->t)
        ret = CompareTimeMetrics(elm1->sb.st_mtim, elm2->sb.st_mtim) * -1;
    else if (usr_opt->c)
        ret = CompareTimeMetrics(elm1->sb.st_ctim, elm2->sb.st_ctim) * -1;
    else if (usr_opt->u)
        ret = CompareTimeMetrics(elm1->sb.st_atim, elm2->sb.st_atim) * -1;
    else
        return strcasecmp(str1, str2);

    if (ret == 0)
        return strcasecmp(str1, str2);

    return ret;
}


void FileListFree(FileList * list)
{
    if (list != NULL)
    {
        FileList *curr = list;
        FileList *next;
        
        while (curr != NULL)
        {
            next = curr->next;
            free(curr->fname);
            free(curr);         
            curr = next;        
        }
    }
}

static int PushToList(char * filename, struct stat * sb, int ishidden, FileList * list)
{
    FileList * elm = calloc(sizeof(FileList), 1);

    if (!elm) 
    {
        FileListFree(list);
        throw_error('\0', filename, MEM_ERR);
        return errno;
    }

    elm->fname = filename;
    elm->ishidden = ishidden;
    elm->sb = *sb;

    while (list->next && (RevSort * FileListCompare(list->next, elm)) < 0)
        list = list->next;

    if (list->next)
    {
        elm->next = list->next;
        list->next = elm;
    }
    else
        list->next = elm;

    return 0;
}

int FileListInsert(char * filename, FileList * filelist, FileList * reclist)
{
    int ishidden = 0;

    /* 
     * If the file is hidden and the -a option is specified print it
     * Else skip it. if the file is not hidden print it anyways
     */
    if (IsHidden(filename))
    {
        if (usr_opt->a)
            ishidden++;
        else
        {
            free(filename);
            return 0;
        }
    }
    
    struct stat sb;

    if (stat(filename, &sb) == -1) 
    {
        throw_error('\0', filename, WRNG_TARG_ERR);
        return errno;
    } 

    if (S_ISDIR(sb.st_mode))
        PushToList(strdup(filename), &sb, ishidden, reclist);

    PushToList(filename, &sb, ishidden, filelist);
 
    return 0;
}

void FileListLog(FileList * list)
{
    int count = 0;
    list = list->next;

    if (list)
    {
        while (list != NULL)
        {

            printf("%s -> ", list->fname);
            count++;
            list = list->next;

        }
    }
    printf("(end)\n\n");
}