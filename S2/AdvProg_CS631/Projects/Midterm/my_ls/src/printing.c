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
#include "padding-handling.h"
#include "opt_parser.h"
#include "utility.h"

#include "printing.h"

extern UsrOptions * USR_OPT;
extern int BLOCK_SIZE;
extern PaddingInfos * PINFOS;


/* 
 * This macro represent the maximum possible len for a
 * Date formated by ls
 * ex "Dec 31 00:00" is 12 char long + the '\0' = 13
 * it can't be anything else as we do not print year
 * (unless if it's a more than a year later but again 5 chars will be changed to 4 max)
 * which is the only field that can possibly grow the string
 * outside his bounds year 9999->10000
 */
#define MAX_DATE_LEN 13

/*
 * This macro represent a year in seconds
 * Nb_days_in_a_year * Nb_hours_in_a_day 
 * 	* Nb_minutes_in_a_hour * nb_seconds_in_a_minutes
 *
 * 365 * 24 * 60 * 60 = 31536000
 *
 * Used to check if a file has been created long ago.
 */
#define YEAR_IN_SECONDS 31536000

#ifndef SIZES
#define SIZES
#define BSIZE 1
#define KBSIZE 1024
#define MBSIZE 1048576
#define GBSIZE 1073741824
#define TBSIZE 1099511627776
#endif /* ! SIZES */

static void
PrintFileName(char * filename)
{
    /* Force printing of non-printable characters as '?' */
    if (USR_OPT->q)
    {
        while (*filename != '\0')
	{
	    if (*filename < ' ' || *filename > '~')
		putc('?', stdout);
	    else
		putc(*filename, stdout);

	    filename++;
	}
    }
    /* Force raw printing of non-printable characters */
    else if (USR_OPT->w)
        fwrite(filename, sizeof(char), strlen(filename), stdout);
    /* Else, print the file name normally */
    else
	printf("%s", filename);
}

void
PrintTotalBytes()
{
    char unit = '\0';
    long double total_p = 0;
   
    /* 
     * If -h is specified then we need to print the total of bytes
     * else it's the total of blocks
     */ 
    if (USR_OPT->h)
    	total_p = PINFOS->total_bytes;
    else
	total_p = PINFOS->total_blocks;

    if (total_p != 0)
    {
        if (total_p < KBSIZE)
	    unit = 'B';
	else if (total_p >= KBSIZE && total_p < MBSIZE)
	    unit = 'K';
	else if (total_p >= MBSIZE && total_p < GBSIZE)
	    unit = 'M';
	else if (total_p >= GBSIZE && total_p < TBSIZE)
	    unit = 'G';
	else if (total_p >= TBSIZE)
	    unit = 'T';
    }

    if (USR_OPT->h)
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
    return EXIT_SUCCESS;
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
    else if (raw_nb_bytes >= KBSIZE && raw_nb_bytes < MBSIZE)
        unit = 'K';
    else if (raw_nb_bytes >= MBSIZE && raw_nb_bytes < GBSIZE)
        unit = 'M';
    else if (raw_nb_bytes >= GBSIZE && raw_nb_bytes < TBSIZE)
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

static int 
PrintOwner(struct stat sb)
{
    if (!USR_OPT->n)
    {
        struct passwd * pwd = getpwuid(sb.st_uid);
	
	if (pwd == NULL)
	{
	    if (PrintIntVal(0, sb.st_uid, PINFOS->max_uid_int_len))
	        return errno;
	}
	else
	{
	    printf("%s", pwd->pw_name);
	    Padding(pwd->pw_name, PINFOS->max_uid_len);
	}
    }
    else
    {
	if (PrintIntVal(0, sb.st_uid, PINFOS->max_uid_int_len))
	    return errno;
    }
    
    printf("  ");
    return EXIT_SUCCESS;
    
}

static int 
PrintGroup(struct stat sb)
{
    if (!USR_OPT->n)
    {
        struct group * grp = getgrgid(sb.st_gid);
	
	if (grp == NULL)
	{
	   if(PrintIntVal(0, sb.st_gid, PINFOS->max_gid_int_len))
	       return errno;
	}
	
	else
	{
	    printf("%s", grp->gr_name);
	    Padding(grp->gr_name, PINFOS->max_gid_len);
	}
    }
    else
    {
	if (PrintIntVal(0, sb.st_gid, PINFOS->max_gid_int_len))
	    return errno;
    }
    
    printf(" ");
    return EXIT_SUCCESS;
}

static void 
PrintDate(struct stat sb)
{
     char out_str[MAX_DATE_LEN] = {'\0'};
     struct tm *info;
     time_t now;
     char * date_to_print;
     int time_err;
     int time_diff;

     tzset();
     info = localtime(&(sb.st_mtime));
     time_err = time(&now);
     time_diff = difftime(now, sb.st_mtime);


     if ((time_err == -1) 
		     || abs((int) time_diff) < YEAR_IN_SECONDS) {
	date_to_print = "%b %e %H:%M";
     } else {
	date_to_print = "%b %e  %Y";
     }

     strftime(out_str, sizeof(out_str), date_to_print, info);
     printf("%s", out_str);
}

/* Add a character representing the file type after the file name */
static void
Handle_F_Option(struct stat sb) {
    if (USR_OPT->F)
    {
	char filetype;
	char * res = calloc(sizeof(char), 11);
	strmode(sb.st_mode, res);
	
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
		if (sb.st_mode & S_IXUSR)
			printf("*");
		break;
	    default:
		break;
	}
    }
}


static int 
Handle_i_Option(struct stat sb)
{
    if (USR_OPT->i)
    {
        if (PrintIntVal(1, sb.st_ino, PINFOS->max_inode_nb_len))
		return errno;
    	printf(" ");
    }
    return EXIT_SUCCESS;
}

static int 
Handle_s_Option(struct stat sb, int fromblocks)
{
    if (USR_OPT->s)
    {
         if (!USR_OPT->h)
         {
             if(PrintIntVal(1, ComputeBlock(sb.st_blocks), PINFOS->max_nb_block_len))
	         return errno;
	 }
	 else
	 {
	     long double raw_bytes = 0;
 	     if (fromblocks)
	         raw_bytes = sb.st_blocks * 512;
	     else
		 raw_bytes = sb.st_size;

	     double computed_bytes = ComputeBytes(raw_bytes);
	     PrintBytes(computed_bytes, raw_bytes, 0);
	 } 
	 printf(" ");
    }
    return EXIT_SUCCESS;
}

static int 
Handle_l_Option(struct stat sb)
{
	if (USR_OPT->l) {
		/* Print the file mode */
		char * res = calloc(sizeof(char), 11);
		strmode(sb.st_mode, res);
		printf("%s ", res);
		free(res);

		/* Print the number of links */
		if (PrintIntVal(1, sb.st_nlink, PINFOS->max_link_nb_len))
		    return errno;
		printf(" ");


		PrintOwner(sb);
		PrintGroup(sb);
		printf(" ");
	
		/* If s or h is specified print the number of bytes */
		if (USR_OPT->s && USR_OPT->h)
	    		PrintBytes(ComputeBytes(sb.st_size), sb.st_size, 0);

		else if (PrintIntVal(1, sb.st_size, PINFOS->max_nb_byte_len))
	    		return errno;

		printf(" ");

		/* Print the date */
		PrintDate(sb);
		printf(" ");
	}
	return EXIT_SUCCESS;
}

static int 
DispFile(struct stat file_sb, char * filename)
{

	if (Handle_i_Option(file_sb))
	    return errno;

	if (Handle_s_Option(file_sb, 1))
	    return errno;

	if (Handle_l_Option(file_sb))
	    return errno;
	
	/* Print the file name */
	PrintFileName(filename);

	Handle_F_Option(file_sb);

	/* If the file is a simlink, print the link path as well */
	if ((USR_OPT->l || USR_OPT->F) && S_ISLNK(file_sb.st_mode)) 
	{
		char linkpath[PATH_MAX] = {0};

    		int linklen = readlink(filename, 
				linkpath, sizeof(linkpath) - 1);

        	linkpath[linklen] = '\0'; 
       		printf(" -> %s", linkpath);
    		 
	}	

        printf("\n");
    
    /* Reset padding for further prints */
    return EXIT_SUCCESS;
}


/* Call DispFile on every file after computing padding necessary */
int LongFormatPrinter(FTSENT *parentdir, FTSENT *list)
{
	FTSENT * saved = list;
	while (saved != NULL) {
		ComputePaddingNeeded(*(list->fts_statp), USR_OPT);
		saved = saved->fts_link;
	}

	while (list != NULL) {
		if (parentdir == NULL && list->fts_info == FTS_D) {
			list = list->fts_link;
			continue;
		}
		if (DispFile(*(list->fts_statp), list->fts_name)) {
			return errno;
		}
		list = list->fts_link;
	}

	return EXIT_SUCCESS;
}

