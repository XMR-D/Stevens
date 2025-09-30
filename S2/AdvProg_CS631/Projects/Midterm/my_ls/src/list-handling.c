#include <sys/stat.h> 

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>  
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
extern int symredirection;

/* PrintInfos structure that will be used later in printing.c */
extern PrintInfos * PINFOS;

void
ResetPrintInfos(PrintInfos * infos)
{
    infos->max_link_nb_len = 0;
    infos->max_uid_len = 0;
    infos->max_gid_len = 0;
    infos->max_uid_int_len = 0;
    infos->max_gid_int_len = 0;
    infos->max_nb_byte_len = 0;
    infos->max_nb_block_len = 0;

    infos->max_inode_nb_len = 0;

    infos->total_bytes = 0;
    infos->total_blocks = 0; 
}

void 
ComputePaddingNeeded(FileList * elm)
{
    
    /* 
     * Compute the length of all the possible numerical fields 
     * of the long format printer for all elments.
     *
     * Compare it to the pinfos actual structure that needs to contain
     * all the max lengths to get a proper output
     */
    struct passwd * pwd = getpwuid(elm->sb.st_uid);
    struct group * grp = getgrgid(elm->sb.st_gid);

    int uid_int_len = NbDigitFromInt(elm->sb.st_uid);
    int gid_int_len = NbDigitFromInt(elm->sb.st_gid);

    int nb_byte_len = NbDigitFromInt(elm->sb.st_size);
    int nb_link_len = NbDigitFromInt(elm->sb.st_nlink);
    int nb_block_len = NbDigitFromInt(ComputeBlock(elm->sb.st_blocks));
    int inode_nb_len = NbDigitFromInt(elm->sb.st_ino);

    if (pwd != NULL)
    {
	int uidlen = strlen(pwd->pw_name);
        if (uidlen > PINFOS->max_uid_len)
	    PINFOS->max_uid_len = uidlen;
    }
    if (grp != NULL)
    {
	int gidlen = strlen(grp->gr_name);
	if (gidlen > PINFOS->max_gid_len)
	    PINFOS->max_gid_len = gidlen;
   }
    
    if (uid_int_len > PINFOS->max_uid_int_len)
        PINFOS->max_uid_int_len = uid_int_len;

    if (gid_int_len > PINFOS->max_gid_int_len)
	PINFOS->max_gid_int_len = gid_int_len;

    if (nb_byte_len > PINFOS->max_nb_byte_len)
	PINFOS->max_nb_byte_len = nb_byte_len;

    if (nb_link_len > PINFOS->max_link_nb_len)
	PINFOS->max_link_nb_len = nb_link_len;

    if (nb_block_len > PINFOS->max_nb_block_len)
        PINFOS->max_nb_block_len = nb_block_len;

    if (inode_nb_len > PINFOS->max_inode_nb_len)
	PINFOS->max_inode_nb_len = inode_nb_len;

    if (usr_opt->h)
        PINFOS->total_bytes += elm->sb.st_size;
    else
        PINFOS->total_blocks += elm->sb.st_blocks;
}

static int 
FileListCompare(FileList * elm1, FileList * elm2)
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


void 
FileListFree(FileList * list)
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

static int 
PushToList(char * filename, struct stat * sb, 
		int ishidden, int padding, FileList * list)
{
    FileList * elm = calloc(sizeof(FileList), 1);

    if (!elm) 
    {
        FileListFree(list);
        throw_error(filename, MEM_ERR);
        return errno;
    }

    elm->fname = filename;
    elm->ishidden = ishidden;
    elm->sb = *sb;

    if (padding)
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

int FileListInsert(char * dirname, char * filename, 
		FileList * filelist, FileList * reclist)
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
    int fullpath_len = dir_len + file_len + 2;

    char * fullpath = malloc(dir_len + file_len + 2);
    if (fullpath == NULL)
    	return errno;

    snprintf(fullpath, fullpath_len, "%s/%s", dirname, filename);

    if (fstatat(AT_FDCWD, fullpath, &sb, AT_SYMLINK_NOFOLLOW) == -1) 
    {
        throw_error(fullpath, WRNG_TARG_ERR);
	free(fullpath);
        return errno;
    } 

    if (S_ISDIR(sb.st_mode) && !usr_opt->d)
    {
        if (PushToList(strdup(filename), &sb, ishidden, 0, reclist) != 0)
	    return errno;
    }

    if (PushToList(strdup(filename), &sb, ishidden, 1, filelist) != 0)
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
