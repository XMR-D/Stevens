#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "error.h"
#include "list-handling.h"
#include "ls.h"
#include "opt_parser.h"
#include "printing.h"
#include "targ_parser.h"
#include "utility.h"

#include "listing.h"


PrintInfos * pinfos;

extern UsrOptions * usr_opt;
extern int targ_count;
extern int in_recursion;


void
ResetPinfos(PrintInfos * infos)
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

/* List files from dir and create a filelist and reclist for last step (STEP 3 PRINTING) */
int ListFile(char * dir, FileList * filelist, FileList * reclist)
{
    DIR * dp;
    struct dirent * dirp;


    if((dp = opendir(dir)) == NULL)
    {
        throw_error(dir, WRNG_TARG_ERR);
        return errno;
    }


    while((dirp = readdir(dp)) != NULL)
    {
        if ((strncmp(dirp->d_name, "..", 2) == 0 || strncmp(dirp->d_name, ".", 1) == 0) 
			&& !usr_opt->a)
            continue;
	
        if (FileListInsert(dir, dirp->d_name, filelist, reclist) != 0)
	    return errno;
    }

    closedir(dp);
    return 0;
}

int TargetLProcess(TargList * targets)
{
    pinfos = calloc(sizeof(PrintInfos), 1);
    if (!pinfos)
    {
	throw_error("", MEM_ERR);
	return errno;
    }

    targets = targets->next;

    while(targets != NULL)
    {
        if (targets->isdir)
        {
	    /* 
	     * Fore more output clarity, if we have more than one target
	     * print the name of the target then the listing
	     */
	    if (targ_count > 1)
            	printf("\n%s :\n", targets->target);
	    
	    /* 
	     * We have a directory to read through and list
	     * so we create a new directory listing list
	     * and a recursion list (in case of -R), Could have done 
	     * that better but I was feeling lazy
	     */

            FileList * dir_listing = calloc(sizeof(FileList), 1);
            if (!dir_listing)
            {
                throw_error(NULL, MEM_ERR);
                return errno;
            }

            FileList * new_targets = calloc(sizeof(FileList), 1);
            if (!new_targets)
            {
                free(dir_listing);
                throw_error(NULL, MEM_ERR);
                return errno;
            }

            if(ListFile(targets->target, dir_listing, new_targets))
            {
                FileListFree(dir_listing);
                FileListFree(new_targets);
                return errno;
            }
            
	    if (LongFormatPrinter(dir_listing))
	    {
                FileListFree(dir_listing);
		FileListFree(new_targets);
		return errno;
	    }

            FileListFree(dir_listing);

	    if (usr_opt->R)
	    {	
		FileList * curr = new_targets;

		/* Skip head */
		curr = curr->next;

		while (curr != NULL)
		{
		    /* Call ls on each new targets so one at a time */

		    /* Prepare new_arguments */
		    int argc = 2;
		    char ** argv = calloc(sizeof(char *), 2);

		    /* Craft path relative to the actual target */
		    int path_len = strlen(curr->fname) + strlen(targets->target) + 2;
		    char * new_path = calloc(sizeof(char), path_len);
		    snprintf(new_path, path_len, "%s/%s", targets->target, curr->fname);

		    printf("\n");

		    argv[0] = "ls";
		    argv[1] = new_path;

		    /* Specify that fetching options is no more necessary*/
		    in_recursion = 1;

		    if (ls_main(argc, argv))
		    {
			FileListFree(new_targets);
			return errno;
		    }
		    curr = curr->next;
		}
	    }

	    FileListFree(new_targets);
	    targets = targets->next;
        }
        else
        {
  	    /*
	     * The target is a file, normally at this stage all
	     * files are placed before any directories
	     * So this code is safe, even if we calloc and
	     * handle a new list made for file targets
	     */
	    FileList * file_listing = calloc(sizeof(FileList), 1);
            
            while(targets != NULL && !targets->isdir)
            {
                char * filename = strdup(targets->target);
                FileListInsert("./", filename, file_listing, NULL); 
                targets = targets->next;
            }

	    if (LongFormatPrinter(file_listing))
	    {
		FileListFree(file_listing);
		return errno;
	    }

            FileListFree(file_listing);
        }


	if (targets == NULL)
	{
	    free(pinfos);
            return 0;
	}
	else
	   printf("\n");
    }
    free(pinfos);
    return 0;
}
