#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "error.h"
#include "list-handling.h"
#include "opt_parser.h"
#include "targ_parser.h"
#include "utility.h"


#include "listing.h"

extern int target_count;
int header_print;

/* List files from dir and create a filelist and reclist for last step (STEP 3 PRINTING) */
int ListFile(char * dir, FileList * filelist, RecList * reclist)
{
    DIR * dp;
    if((dp = opendir(dir)) == NULL)
    {
        /* TODO THROW AN ERROR HERE */
        throw_error('\0', dir, WRNG_TARG_ERR);
        return errno;
    }

    struct dirent * dirp;


    while((dirp = readdir(dp)) != NULL)
    {
        if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0)
            continue;

        FileListInsert(FullName(dir, dirp->d_name), filelist, reclist);

    }
    closedir(dp);
    return 0;
}

int TargetLProcess(TargList * targ_list)
{
    TargList * list = targ_list->next;

    while(list)
    {
        if (list->isdir)
        {
            printf("%s :\n", list->target);

            FileList * files = calloc(sizeof(FileList), 1);
            RecList * new_targets = calloc(sizeof(RecList), 1);

            if(ListFile(list->target, files, new_targets))
            {
                RecListFree(new_targets);
                FileListFree(files);
                return errno;
            }
            FileListLog(files);
            RecListLog(new_targets);
            
            /* 
                HERE WE SHOULD HAVE FILES LIST AND IF -R OPT IS SPECIFIED A RECURSION LIST
                IN THIS CASE RELAUNCH HERE LS LIKE (ls list->target/new_target)
                YES WE NEED TO CONCAT THE ACTUAL TARGET AND THE NEW FOUND TARGET.
            */

            FileListFree(files);
            RecListFree(new_targets);


        }
        else
        {
            /* ITS A FILE, PRINT IT IN ACCORDANCE WITH OPTION */
            printf("%s\n", list->target);
        }
        list = list->next;
    }
    return 0;
}