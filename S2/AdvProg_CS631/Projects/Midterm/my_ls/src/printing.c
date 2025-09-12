#include <sys/ioctl.h>
#include <sys/stat.h>

#include <errno.h>
#include <math.h>
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

extern long double total_blocks;
extern long double total_bytes;
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

#ifndef SIZES
#define SIZES
#define BSIZE 1
#define KBSIZE 1024
#define MBSIZE 1048576
#define GBSIZE 1073741824
#define TBSIZE 1099511627776
#endif /* ! SIZES */

static void
PrintTotalBytes()
{
    char unit = '\0';
    long double total_p = 0;
   
    /* 
     * If -h is specified then we need to print the total of bytes
     * else it's the total of blocks
     */ 
    if (usr_opt->h)
    	total_p = total_bytes;
    else
	total_p = total_blocks;

    if (total_p != 0)
    {
        if (total_p < (long double) KBSIZE)
	    unit = 'B';
	else if (total_p >= (long double) KBSIZE)
	    unit = 'K';
	else if (total_p >= (long double) MBSIZE)
	    unit = 'M';
	else if (total_p >= (long double) GBSIZE)
	    unit = 'G';
	else if (total_p >= (long double) TBSIZE)
	    unit = 'T';
    }

    if (usr_opt->h)
    {
	long double bytes_to_print = ComputeBytes(total_p);
	if (bytes_to_print >= 10.0f)
	    printf("total %i%c\n", (int) round(bytes_to_print), unit);
	else
            printf("total %.1Lf%c\n", bytes_to_print, unit);
    }
    else
    	printf("total %i\n", ComputeBlock((int) total_p));
}

/* If padding_order = 0 the padding is after the value, else it's before */
static int 
PrintIntVal(int padding_order, long int val, long int max_len)
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

static void 
PrintBytes(double nb_bytes, long int raw_nb_bytes, int do_round)
{
    char pbuf[5] = {0};

    char unit = '\0';

    if (nb_bytes == 0)
    {
        printf("  0B");
	return;
    } 

    if (raw_nb_bytes < KBSIZE)
        unit = 'B';
    else if (raw_nb_bytes >= KBSIZE)
        unit = 'K';
    else if (raw_nb_bytes >= MBSIZE)
        unit = 'M';
    else if (raw_nb_bytes >= GBSIZE)
        unit = 'G';
    else if (raw_nb_bytes >= TBSIZE)
        unit = 'T';
    

    if (nb_bytes >= 10.f)
    {
	if (do_round)
    	    snprintf(pbuf, 5, "%i%c", (int) round(nb_bytes), unit);
	else
    	    snprintf(pbuf, 5, "%i%c", (int) nb_bytes, unit);
	    
    }	
    else
	snprintf(pbuf, 5, "%.1f%c", nb_bytes, unit);

    Padding(pbuf, 4);
    printf("%s", pbuf);
    
}

int Handle_s_Option(FileList * elm, int fromblocks)
{
    if (usr_opt->s)
    {
         if (!usr_opt->h)
         {
             if(PrintIntVal(1, ComputeBlock(elm->sb.st_blocks), max_nb_block_len))
	         return errno;
	 }
	 else
	 {
	     long double raw_bytes = 0;
 	     if (fromblocks)
	         raw_bytes = elm->sb.st_blocks * 512;
	     else
		 raw_bytes = elm->sb.st_size;

	     double computed_bytes = ComputeBytes(raw_bytes);
	     PrintBytes(computed_bytes, raw_bytes, 0);
	 } 
	 printf(" ");
    }
    return 0;
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

static int 
PrintOwner(FileList * elm)
{
    if (!usr_opt->n)
    {
        struct passwd * pwd = getpwuid(elm->sb.st_uid);
	
	if (pwd == NULL)
	{
	    if (PrintIntVal(0, elm->sb.st_uid, max_uid_int_len))
	        return errno;
	}
	else
	{
	    printf("%s", pwd->pw_name);
	    Padding(pwd->pw_name, max_uid_len);
	}
    }
    else
    {
	if (PrintIntVal(0, elm->sb.st_uid, max_uid_int_len))
	    return errno;
    }
    
    printf("  ");
    return 0;
    
}

static int 
PrintGroup(FileList * elm)
{
    if (!usr_opt->n)
    {
        struct group * grp = getgrgid(elm->sb.st_gid);
	
	if (grp == NULL)
	{
	   if(PrintIntVal(0, elm->sb.st_gid, max_gid_int_len))
	       return errno;
	}
	
	else
	{
	    printf("%s", grp->gr_name);
	    Padding(grp->gr_name, max_gid_len);
	}
    }
    else
    {
	if (PrintIntVal(0, elm->sb.st_gid, max_gid_int_len))
	    return errno;
    }
    
    printf(" ");
    return 0;
}


static void 
PrintDate(FileList * elm)
{
     char out_str[MAX_DATE_LEN] = {'\0'};
     struct tm *info = localtime(&(elm->sb.st_mtime));

     strftime(out_str, sizeof(out_str), "%b %e %H:%M", info);
     printf("%s", out_str);
}

int 
LongFormatPrinter(FileList * list)
{
    /* Skip head element */
    list = list->next;

    if (usr_opt->s)
        PrintTotalBytes();

    while (list)
    {
	if (Handle_s_Option(list, 1))
	    return errno;

	PrintFileMode(list);
	printf("  ");

	/* number of links */
	if (PrintIntVal(1, list->sb.st_nlink, max_link_nb_len))
	    return errno;

	printf(" ");
	PrintOwner(list);
	PrintGroup(list);

	/* Number of bytes */
	printf(" ");
	if (usr_opt->s && usr_opt->h)
	    PrintBytes(ComputeBytes(list->sb.st_size), list->sb.st_size, 0);

	else if (PrintIntVal(1, list->sb.st_size, max_nb_byte_len))
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

    total_blocks = 0;
    total_bytes = 0;

    return 0;
}

