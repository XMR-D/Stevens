#include <sys/stat.h> 

#include <errno.h>
#include <pwd.h>
#include <grp.h>  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "error.h"
#include "opt_parser.h"
#include "utility.h"

#include "list-handling.h"

extern int RevSort;
extern UsrOptions * usr_opt;

/* 
 * This set of global variable is used to compute the padding 
 * When printing in long format
 *
 * NEED TO CHANGE THAT TO A STRUCT
 */
int max_link_nb = 0;
int max_uid_len = 0;
int max_gid_len = 0;
int max_uid_int_len = 0;
int max_gid_int_len = 0;
int max_nb_byte_len = 0;
int total_bytes = 0;

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

    /* The element is a file and need to be sorted */
    
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
            
	    if(curr->fname)
	        free(curr->fname);
            
	    free(curr);         
            curr = next;        
        }
    }
}

static void ComputePaddingNeeded(FileList * elm)
{
    struct passwd * pwd = getpwuid(elm->sb.st_uid);
    struct group * grp = getgrgid(elm->sb.st_gid);
    int uid_int_len = NbDigit(elm->sb.st_uid);
    int gid_int_len = NbDigit(elm->sb.st_gid);

    int nb_byte_len = NbDigit(elm->sb.st_size);
    int nb_link_len = NbDigit(elm->sb.st_nlink);

    if (pwd != NULL)
    {
	int uidlen = strlen(pwd->pw_name);
        if (uidlen > max_uid_len)
	    max_uid_len = uidlen;
    }
    if (grp != NULL)
    {
	int gidlen = strlen(grp->gr_name);
	if (gidlen > max_gid_len)
	    max_gid_len = gidlen;
    }
    
    if (uid_int_len > max_uid_int_len)
	    max_uid_int_len = uid_int_len;

    if (gid_int_len > max_gid_int_len)
	    max_gid_int_len = gid_int_len;

    if (nb_byte_len > max_nb_byte_len)
	    max_nb_byte_len = nb_byte_len;

    if (nb_link_len > max_link_nb)
	    max_link_nb = nb_link_len;

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

    ComputePaddingNeeded(elm);

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

int FileListInsert(char * dirname, char * filename, FileList * filelist, FileList * reclist)
{
    int ishidden = 0;

    /* 
     * If the file is hidden and the -a option is specified print it
     * Else skip it. if the file is not hidden print it anyways
     */
    if (IsHidden(filename))
        ishidden++;
    
    struct stat sb;

    int dir_len = strlen(dirname);
    int file_len = strlen(filename);

    char * fullpath = malloc(dir_len + file_len + 2);
    if (fullpath == NULL)
    	return errno;
    snprintf(fullpath, dir_len + file_len + 2, "%s/%s", dirname, filename);

    if (stat(fullpath, &sb) == -1) 
    {
        throw_error('\0', fullpath, WRNG_TARG_ERR);
	free(fullpath);
        return errno;
    } 

    if (S_ISDIR(sb.st_mode))
    {
        if (PushToList(strdup(filename), &sb, ishidden, reclist) != 0)
	    return errno;
    }

    if (PushToList(strdup(filename), &sb, ishidden, filelist) != 0)
        return errno;

    free(fullpath);
 
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
