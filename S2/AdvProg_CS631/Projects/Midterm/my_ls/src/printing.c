#include <sys/ioctl.h>
#include <sys/stat.h>

#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "list-handling.h"
#include "opt_parser.h"
#include "utility.h"

#include "printing.h"

extern UsrOptions * usr_opt;

extern int max_link_nb_len;
extern int max_uid_len;
extern int max_gid_len;
extern int max_uid_int_len;
extern int max_gid_int_len;
extern int max_nb_byte_len;
extern int max_nb_block_len;

extern int total_bytes;
extern int block_size;

/* 
 * This macro represent the maximum possible len for a
 * Date formated by ls
 * ex "Dec 31 00:00" is 12 char long + the '\0' = 13
 * it can't be anything else as we do not print year
 * which is the only field that can possibly grow the string
 * outside his bounds year 9999->1000000
 */
#define MAX_DATE_LEN 13

/* If padding_order = 0 the padding is after the value, else it's before */
static int 
PrintNumVal(int padding_order, long int val, long int max_len)
{
    char * val_str = calloc(sizeof(char), NbDigit(val) + 1);
    if (!val_str)
    {
        throw_error("", MEM_ERR);
	return errno;
    }

    sprintf(val_str, "%ld", val);

    if (padding_order)
    	Padding(val_str, max_len);
    
    printf("%s", val_str);

    if (padding_order == 0)
	Padding(val_str, max_len);
    
    free(val_str);
    return 0;
}

/* TODO DEBUG THIS LAST STEP + ADD options handling */
static void PrintListing(FileList * list, int longest, int row_nb, int col_nb)
{
    /* Setup the start of the list (first elm) */
    if (!list->fname && list->next)
        list = list->next;

    FileList * start = list;
    FileList * curr = list;
    int step = row_nb;
    int new_row = 0;

    /* 
     * If the list contain only one element 
     * Print it, and return
     */
    if (!curr->next)
    {
    	printf("%s\n", curr->fname);
	return;
    }
    
    if (usr_opt->s)
    {
	if (!usr_opt->h)
	   PrintNumVal(1, ComputeBlock(curr->sb.st_blocks), max_nb_block_len);

	printf(" ");
    }

    printf("%s", curr->fname);
    Padding(curr->fname, longest);
    printf(" ");

    /* 
     * for each row, and for each col print one element
     */

    for (int i = 0; i < row_nb; i++)
    {
	if (new_row)
	{
	    if (usr_opt->s)
	    {
	    	if (!usr_opt->h)
	    	     PrintNumVal(1, ComputeBlock(curr->sb.st_blocks), max_nb_block_len);

	        printf(" ");
	    }
	    
	    printf("%s", curr->fname);
	    Padding(curr->fname, longest);
	    printf(" ");
	    new_row--;
	}

        for (int j = 0; j < col_nb; j++)
        {
           
	    /* Shift to next element to print in the linked list */	
            while (step > 0)
            {
                if (!curr->next)
                    break;

                curr = curr->next;
                step--;
            }
	    
            /* 
	     * If on the last element after shifting
	     * and that we do not need to shift anymore print
	     * and proceed to the next row
	     */	    
            if (!curr->next && step == 0)
	    {
		if (usr_opt->s)
	    	{
	    	    if (!usr_opt->h)
	    	        PrintNumVal(1, ComputeBlock(curr->sb.st_blocks), max_nb_block_len);

	            printf(" ");
	    	}

		printf("%s", curr->fname);
		break;
	    }

	    /*
	     * If after shifting we found an element print it with padding
	     * and reset the step for the next_path
	     */
            if (curr->next)
	    {
		if (usr_opt->s)
	    	{
	    	    if (!usr_opt->h)
	    	        PrintNumVal(1, ComputeBlock(curr->sb.st_blocks), max_nb_block_len);

	            printf(" ");
	    	}
		step = row_nb;
                printf("%s", curr->fname);
                Padding(curr->fname, longest);
		printf(" ");
	    }

        }
	new_row++;
	step = row_nb; 
        start = start->next;
        curr = start;
        printf("\n");
    }
}


static int GetWinWidth(void)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
}

static int GetMaxLen(FileList * list, int * nb_files)
{
    int max = 0;
    int curr_len = 0;

    if (!list->fname && list->next)
        list = list->next;

    while (list)
    {	
	curr_len = strlen(list->fname);
        
	(*nb_files)++;

        if (max < curr_len)
            max = curr_len;

        list = list->next;
    }
    return max;
}


void ClassicPrinter(FileList * list)
{
    /* If the list is empty (no files to print) return */
    if (!list->next)
        return;
    /* 
     * Compute rows and columns to print the files
     * The width of a column is the size of the largest file + 2 for padding
     * Given by GetMaxLen(list);
     * Get the width of the terminal to compute how to print the output
     * Given by GetWinWidth();
    */

    int nb_files = 0;
    int longest = GetMaxLen(list, &nb_files);
    int col_nb = GetWinWidth() / longest;
    int row_nb = (nb_files + col_nb - 1) / col_nb;
    PrintListing(list, longest, row_nb, col_nb);
}

static void PrintFileType(FileList * elm)
{
    if (S_ISWHT(elm->sb.st_mode))
        printf("w");

    else if (S_ISREG(elm->sb.st_mode))
    {
	//TODO CHANGE THESES TO CORRETLY PRINT A and a
	if (elm->sb.st_flags & SF_ARCHIVED)
	    printf("a");
	if (elm->sb.st_flags & SF_ARCHIVED)
	    printf("A");
	else
	    printf("-");
    }
    else if (S_ISDIR(elm->sb.st_mode))
        printf("d");
    else if (S_ISCHR(elm->sb.st_mode))
        printf("c");
    else if (S_ISBLK(elm->sb.st_mode))
        printf("b");
    else if (S_ISFIFO(elm->sb.st_mode))
        printf("p");
    else if (S_ISLNK(elm->sb.st_mode))
        printf("l");
    else if (S_ISSOCK(elm->sb.st_mode))
	printf("s");
    
    return;
}

static void PrintPermission(FileList *elm, int field, char * permission)
{
     //TODO : ADD STICKY BIT AND Set user ID mode
     if (elm->sb.st_mode & field)
         printf("%s", permission);
     else
	 printf("-");
}

static void PrintFileMode(FileList * elm)
{
    /* file type */
    PrintFileType(elm);
    
    /* owner permissions */
    PrintPermission(elm, S_IRUSR, "r");	
    PrintPermission(elm, S_IWUSR, "w");	
    PrintPermission(elm, S_IXUSR, "x");	
    
    /* group permissions */
    PrintPermission(elm, S_IRGRP, "r");	
    PrintPermission(elm, S_IWGRP, "w");	
    PrintPermission(elm, S_IXGRP, "x");

    /* other permissions */    
    PrintPermission(elm, S_IROTH, "r");	
    PrintPermission(elm, S_IWOTH, "w");	
    PrintPermission(elm, S_IXOTH, "x");

}

static void PrintOwner(FileList * elm)
{
    if (!usr_opt->n)
    {
        struct passwd * pwd = getpwuid(elm->sb.st_uid);
	
	if (pwd == NULL)
	    PrintNumVal(0, elm->sb.st_uid, max_uid_int_len);
	else
	{
	    printf("%s", pwd->pw_name);
	    Padding(pwd->pw_name, max_uid_len);
	}
    }
    else
	PrintNumVal(0, elm->sb.st_uid, max_uid_int_len);
    
    printf("  "); 
    
}

static void PrintGroup(FileList * elm)
{
    if (!usr_opt->n)
    {
        struct group * grp = getgrgid(elm->sb.st_gid);
	
	if (grp == NULL)
	   PrintNumVal(0, elm->sb.st_gid, max_gid_int_len);
	
	else
	{
	    printf("%s", grp->gr_name);
	    Padding(grp->gr_name, max_gid_len);
	}
    }
    else 
	PrintNumVal(0, elm->sb.st_gid, max_gid_int_len);
    
    printf(" ");
}


static void PrintDate(FileList * elm)
{
     char out_str[MAX_DATE_LEN] = {'\0'};
     struct tm *info = localtime(&(elm->sb.st_mtime));

     strftime(out_str, sizeof(out_str), "%b %e %H:%M", info);
     printf("%s", out_str);

}

int LongFormatPrinter(FileList * list)
{
    /* Skip head element */
    list = list->next;

    while (list)
    {
	/*TODO: here add inode number, again with padding */
	if (usr_opt->s)
	{
	    if (!usr_opt->h)
	    	PrintNumVal(1, ComputeBlock(list->sb.st_blocks), max_nb_block_len);

	    printf(" ");
	}
	
	PrintFileMode(list);
	printf("  ");

	/* number of links */
	if (PrintNumVal(1, list->sb.st_nlink, max_link_nb_len))
	    return errno;

	printf(" ");
	PrintOwner(list);
	PrintGroup(list);

	/* Number of bytes */
	printf(" ");
	if (PrintNumVal(1, list->sb.st_size, max_nb_byte_len))
	    return errno;

	printf(" ");

	PrintDate(list);
	printf(" ");
	printf("%s\n", list->fname);
        
	list = list->next;
    }
    
    /* Reset padding for further prints */
    max_uid_len = 0;
    max_gid_len = 0;
    max_uid_int_len = 0;
    max_gid_int_len = 0;
    max_nb_byte_len = 0;

    return 0;
}

