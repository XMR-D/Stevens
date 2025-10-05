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
extern int rec_level;

extern char * TARGET_PATH;

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

	    if (IsDotDirectory(curr->fname)) {
		curr = curr->next;
	        continue;
	    }
  
	    argc = 2;
	    argv = calloc(sizeof(char *), 2);
	    if (argv == NULL) {
	        throw_error(NULL, MEM_ERR);
		return errno;
	    }

	    argv[0] = "ls";
	    argv[1] = curr->fname;	     
	    
	    rec_level++;
	    
	    if (ls_main(argc, argv)) {
		free(argv);
		return errno;
	    }

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
	/* if -A is specified and the file is . or .. skip it. */
	if (usr_opt->A && !usr_opt->a) {
		if (IsDotDirectory(dirp->d_name)) {
			continue;
		}
	} 
	/* if the file is starting with '.' and -a is not specified skip */
	if (IsStartingWithDot(dirp->d_name)) {
		if (!usr_opt->a && !usr_opt->A) {
			continue;
		}
	}
		
        if (FileListInsert(dir, dirp->d_name, filelist, reclist) != 0) {
	    return errno;
	}
    }

    closedir(dp);
    return 0;
}


int
DirectoriesProcess(TargList * directories_list)
{
    while(directories_list != NULL) {
	    /* 
	     * Fore more output clarity, if we have more than one target
	     * print the name of the target then the listing
	     */
	    if (targ_count > 1) {
		TARGET_PATH = ConcatTargetToPath(TARGET_PATH, 
				directories_list->target);

		printf("\n%s:\n", TARGET_PATH);
	    }
	    
	    /* 
	     * We have a directory to read through and list
	     * so we create a new directory listing list
	     * and a recursion list (in case of -R)
	     */

            FileList * dir_listing = calloc(sizeof(FileList), 1);
            FileList * new_targets = calloc(sizeof(FileList), 1);
            if (!dir_listing || !new_targets) {

		if (dir_listing != NULL) {
		    FileListFree(dir_listing);	
		}
		if (new_targets != NULL) {
		    FileListFree(new_targets);
		}

                throw_error(NULL, MEM_ERR);
                return errno;
            }

	    /* 
	     * Attempt to list then to print the files, errors will be catched
	     * in the case one fail.
	     */
            if(ListFile(directories_list->target, dir_listing, new_targets)
			    || LongFormatPrinter(dir_listing)) {
                FileListFree(dir_listing);
                FileListFree(new_targets);
                return errno;
            }
            
	    if (usr_opt->R) {
		if (Handle_R_option(directories_list, new_targets)) {
            	    FileListFree(dir_listing);
		    FileListFree(new_targets);
		    return errno;
		}
	    }
	    

            FileListFree(dir_listing);
	    FileListFree(new_targets);
	    directories_list = directories_list->next;
    	    ResetPrintInfos(PINFOS);
	    TARGET_PATH = RemoveLastFileFromPath(TARGET_PATH);
    }

    return EXIT_SUCCESS;

}

int 
FilesProcess(TargList * files_list)
{

    FileList * file_listing = calloc(sizeof(FileList), 1);
            
    while(files_list != NULL) {
    	char * filename = strdup(files_list->target);
        if (FileListInsert("./", filename, file_listing, NULL)) {
		//HANDLE ERROR
	} 
	free(filename);
        files_list = files_list->next;
    }
    
    if (LongFormatPrinter(file_listing)) {
	FileListFree(file_listing);
	return errno;
    }

    FileListFree(file_listing);
    ResetPrintInfos(PINFOS);
    return errno; 
}
