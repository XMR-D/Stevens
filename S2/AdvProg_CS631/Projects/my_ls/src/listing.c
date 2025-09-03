#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "opt_parser.h"
#include "targ_parser.h"

#include "listing.h"

extern int target_count;
int header_print;

int ListFile(char * dir)
{
    printf("%s\n", dir);
    return 0;
}

int TargetLProcess(TargList * targ_list)
{
    if (!targ_list)
    {
        throw_error('\0', "(null)", "Empty target_list", WRNG_TARG_ERR);
        return WRNG_TARG_ERR;
    }

    TargList * list = targ_list->next;

    while(list)
    {
        if (list->isdir)
        {
            //FileList * files = malloc(sizeof(FileList));
            //RecList * new_targets = malloc(sizeof(RecList));

            if(ListFile(list->target))
            {
                throw_error('\0', "(null)", "Error encountered during listing in directory", WRNG_TARG_ERR);
                return WRNG_TARG_ERR;
            }

            /* 
                HERE WE SHOULD HAVE FILES LIST AND IF -R OPT IS SPECIFIED A RECURSION LIST
                IN THIS CASE RELAUNCH HERE LS LIKE (ls list->target/new_target)
                YES WE NEED TO CONCAT THE ACTUAL TARGET AND THE NEW FOUND TARGET.
            */
        }
        else
        {
            /* ITS A FILE, PRINT IT IN ACCORDANCE WITH OPTION */
        }
        list = list->next;
    }
    return 0;
}