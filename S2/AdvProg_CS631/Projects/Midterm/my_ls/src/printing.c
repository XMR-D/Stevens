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
#include "listing.h"
#include "list-handling.h"
#include "opt_parser.h"
#include "utility.h"

#include "printing.h"

extern UsrOptions * usr_opt;

extern PrintInfos * pinfos;

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
PrintFileName(FileList * elm)
{
    char * filename = elm->fname;

    /* Force printing of non-printable characters as '?' */
    if (usr_opt->q)
    {
        while (*filename != '\0')
	{
	    if (*filename < 32 || *filename >= 127)
		putc('?', stdout);
	    else
		putc(*filename, stdout);

	    filename++;
	}
    }

    /* Force raw printing of non-printable characters */
    else if (usr_opt->w)
        fwrite(elm->fname, sizeof(char), strlen(elm->fname), stdout);
    /* Else, print the file name normally */
    else
	printf("%s", elm->fname);
    
    /* Add a character representing the file type after the file name */
    if (usr_opt->F)
    {
	char filetype;
	char * res = calloc(sizeof(char), 11);
	strmode(elm->sb.st_mode, res);
	
	filetype = *res;

	switch (filetype)
	{
	    case 'l':
		printf("@");
		break;
	    case 'd':
		printf("/");
	        break;
	    case 's':
		printf("=");
		break;
	    case 'w':
		printf("%c", '%');
		break;
	    case 'p':
		printf("|");
		break;
	    case '-':
		if (elm->sb.st_mode & S_IXUSR)
			printf("*");
		break;
	    default:
		break;
	}
	free(res);
    }
}

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
    	total_p = pinfos->total_bytes;
    else
	total_p = pinfos->total_blocks;

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
    char * val_str = calloc(sizeof(char), NbDigitFromInt(val) + 1);
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

static int Handle_i_Option(FileList * elm)
{
    if (usr_opt->i)
    {
        if (PrintIntVal(1, elm->sb.st_ino, pinfos->max_inode_nb_len))
		return errno;
    	printf(" ");
    }
    return 0;
}

static int Handle_s_Option(FileList * elm, int fromblocks)
{
    if (usr_opt->s)
    {
         if (!usr_opt->h)
         {
             if(PrintIntVal(1, ComputeBlock(elm->sb.st_blocks), pinfos->max_nb_block_len))
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

static int 
PrintOwner(FileList * elm)
{
    if (!usr_opt->n)
    {
        struct passwd * pwd = getpwuid(elm->sb.st_uid);
	
	if (pwd == NULL)
	{
	    if (PrintIntVal(0, elm->sb.st_uid, pinfos->max_uid_int_len))
	        return errno;
	}
	else
	{
	    printf("%s", pwd->pw_name);
	    Padding(pwd->pw_name, pinfos->max_uid_len);
	}
    }
    else
    {
	if (PrintIntVal(0, elm->sb.st_uid, pinfos->max_uid_int_len))
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
	   if(PrintIntVal(0, elm->sb.st_gid, pinfos->max_gid_int_len))
	       return errno;
	}
	
	else
	{
	    printf("%s", grp->gr_name);
	    Padding(grp->gr_name, pinfos->max_gid_len);
	}
    }
    else
    {
	if (PrintIntVal(0, elm->sb.st_gid, pinfos->max_gid_int_len))
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

    if (usr_opt->s || usr_opt->R)
        PrintTotalBytes();

    while (list)
    {
	if (Handle_i_Option(list))
	    return errno;

	if (Handle_s_Option(list, 1))
	    return errno;

	/* Print the file mode */
	char * res = calloc(sizeof(char), 11);
	strmode(list->sb.st_mode, res);
	printf("%s ", res);
	free(res);

	/* Print the number of links */
	if (PrintIntVal(1, list->sb.st_nlink, pinfos->max_link_nb_len))
	    return errno;

	printf(" ");
	PrintOwner(list);
	PrintGroup(list);

	/* If specified by the option print the number of bytes */
	printf(" ");
	if (usr_opt->s && usr_opt->h)
	    PrintBytes(ComputeBytes(list->sb.st_size), list->sb.st_size, 0);

	else if (PrintIntVal(1, list->sb.st_size, pinfos->max_nb_byte_len))
	    return errno;

	printf(" ");

	/* Print the date */
	PrintDate(list);
	printf(" ");

	/* Print the file name */
	PrintFileName(list);
        printf("\n");

	/* then do the same on the next element */
	list = list->next;
    }
    
    /* Reset padding for further prints */
    ResetPinfos(pinfos);
    return 0;
}

