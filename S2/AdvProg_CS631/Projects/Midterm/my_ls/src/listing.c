#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "list-handling.h"
#include "ls.h"
#include "opt_parser.h"
#include "printing.h"
#include "targ_parser.h"
#include "utility.h"

#include "listing.h"


extern PrintInfos * PINFOS;
extern UsrOptions * usr_opt;
extern int targ_count;
extern int root;
extern int rec_level;


/* List files from dir and create a filelist and reclist for last step (STEP 3 PRINTING) */
int 
ListFile(char * dir, FileList * filelist, FileList * reclist)
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
	int ishidden = IsHidden(dirp->d_name);

	/* if the file is . or .. and that a is not specified skip it. */	
        if (ishidden == -1 && !usr_opt->a)
	    continue;

	/* 
	 * if the file is hidden, that we are not root and that the option -a 
	 * is not specified skip it. 
	 */
	if (ishidden == 1 && !root && !usr_opt->a && !usr_opt->A)
	    continue;
		
        if (FileListInsert(dir, dirp->d_name, filelist, reclist) != 0)
	    return errno;
    }

    closedir(dp);
    return 0;
}

static int
Handle_R_option(TargList * targets, FileList *new_targets)
{
	
	int argc;
	char ** argv;
	FileList * curr;

	new_targets = new_targets->next;

	if (new_targets == NULL)
		return EXIT_SUCCESS;

	curr = new_targets;
	

	if (chdir(targets->target))
	{
		throw_error(targets->target, WRNG_TARG_ERR);
		return errno;
	}

	while (curr != NULL)
	{

	    if (IsHidden(curr->fname) == -1)
	    {
		curr = curr->next;
	        continue;
	    }

	    //TODO: - Here save the cwd to print + move accordingly
	    //	    It will help for debugging the ls -R situation
	    //	    
	    //	    - Craft a better saved path, as it will be important 
	    //	    to move between directories 
	    

	    argc = 2;
	    argv = calloc(sizeof(char *), 2);
	    if (argv == NULL)
	    {
	        throw_error(NULL, MEM_ERR);
		return errno;
	    }

	    argv[0] = "ls";
	    argv[1] = curr->fname;	     
	    
	    rec_level++;
	    
	    if (ls_main(argc, argv))
		return errno;

	    free(argv);

	    curr = curr->next;
	    
	}
	
	
	if (rec_level != 0)
	{
		if (chdir(".."))
		{
		     throw_error(targets->target, WRNG_TARG_ERR);
	    	     return errno;
		}
		rec_level--;
	}

	return EXIT_SUCCESS;
}


int 
TargetLProcess(TargList * targ_list)
{
    TargList * targets = targ_list; 

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
            	printf("\n%s:\n", targets->target);
	    
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
                break;
            }

            FileList * new_targets = calloc(sizeof(FileList), 1);
            if (!new_targets)
            {
                throw_error(NULL, MEM_ERR);
                free(dir_listing);
                break;
            }

            if(ListFile(targets->target, dir_listing, new_targets)
			    || LongFormatPrinter(dir_listing))
            {
                FileListFree(dir_listing);
                FileListFree(new_targets);
                break;
            }
            
            FileListFree(dir_listing);

	    if (usr_opt->R)
	    {
		if (Handle_R_option(targets, new_targets))
		{
		    FileListFree(new_targets);
		    break;
		}
	    }
	    
	    FileListFree(new_targets);
	    targets = targets->next;
    	    ResetPrintInfos(PINFOS);
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
		free(filename);
                targets = targets->next;
            }

	    if (LongFormatPrinter(file_listing))
	    {
		FileListFree(file_listing);
		break;
	    }

            FileListFree(file_listing);
        }


	if (targets == NULL)
	{
            return EXIT_SUCCESS;
	}
	else
	   printf("\n");

	ResetPrintInfos(PINFOS);
    }

    ResetPrintInfos(PINFOS);

    if (errno)
        return errno;
    else
    	return EXIT_SUCCESS;
}
