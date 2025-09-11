#include <sys/stat.h>

#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "list-handling.h"
#include "opt-parser.h"
#include "printing.h"
#include "targ-parser.h"
#include "utility.h"

#include "listing.h"

extern int target_count;
int header_print;

/* List files from dir and create a filelist and reclist for last step */
int ListFile(char* dir, FileList* filelist, FileList* reclist) 
{
    DIR* dp;
    if ((dp = opendir(dir)) == NULL) {
        /* TODO THROW AN ERROR HERE */
        throw_error('\0', dir, WRNG_TARG_ERR);
        return errno;
    }

    struct dirent* dirp;

    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0)
            continue;

        FileListInsert(FullName(dir, dirp->d_name), filelist, reclist);
    }
    closedir(dp);
    return 0;
}

int TargetLProcess(TargList* targ_list) 
{
    TargList* list = targ_list->next;

    while (list) {
        if (list->isdir) {
            printf("\n%s :\n", list->target);

            FileList* listing = calloc(sizeof(FileList), 1);
            if (!listing) {
                throw_error('\0', "", MEM_ERR);
                return errno;
            }

            FileList* new_targets = calloc(sizeof(FileList), 1);
            if (!new_targets) {
                free(listing);
                throw_error('\0', "", MEM_ERR);
                return errno;
            }

            if (ListFile(list->target, listing, new_targets)) {
                FileListFree(new_targets);
                FileListFree(listing);
                return errno;
            }

            /*
                HERE WE SHOULD HAVE FILES LIST AND IF -R OPT IS SPECIFIED A
               RECURSION LIST IN THIS CASE RELAUNCH HERE LS LIKE (ls
               list->target/new_target) YES WE NEED TO CONCAT THE ACTUAL TARGET
               AND THE NEW FOUND TARGET.
            */

            ClassicPrinter(listing);

            FileListFree(listing);
            FileListFree(new_targets);
        } else {
            /* TODO :
            * ITS A FILE, PRINT IT IN ACCORDANCE WITH OPTION 
            * Maybe create a FileList that contain all the files to print them
            * using SimpleFilePrinting
            * Then if list->next == NULL or list->next->isdir Print the created
            * list (Classic or Long format) and then free it
            */
            printf("%s  ", list->target);
        }

        list = list->next;
    }
    return 0;
}