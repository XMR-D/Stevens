#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "error.h"
#include "list-handling.h"
#include "opt_parser.h"
#include "printing.h"
#include "targ_parser.h"
#include "utility.h"

#include "listing.h"

extern UsrOptions * usr_opt;
extern int targ_count;

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

int TargetLProcess(TargList * targ_list)
{
    targ_list = targ_list->next;

    while(targ_list)
    {
        if (targ_list->isdir)
        {
	    /* 
	     * Fore more output clarity, if we have more than one target
	     * print the name of the target then the listing
	     */
	    if (targ_count > 1)
            	printf("\n%s :\n", targ_list->target);
	    
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

            if(ListFile(targ_list->target, dir_listing, new_targets))
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
	        //TODO: Handle Recursion
	    }

	    FileListFree(new_targets);
	    targ_list = targ_list->next;
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
            
            while(targ_list != NULL && !targ_list->isdir)
            {
                char * filename = strdup(targ_list->target);
                FileListInsert("./", filename, file_listing, NULL); 
                targ_list = targ_list->next;
            }

	    if (LongFormatPrinter(file_listing))
	    {
		FileListFree(file_listing);
		return errno;
	    }

            FileListFree(file_listing);
        }

	if (targ_list == NULL)
            return 0;
	else
	   printf("\n");
    }
    return 0;
}
