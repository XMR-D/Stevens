#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "opt_parser.h"
#include "utility.h"

#ifndef SIZES
#define BSIZE 1
#define KBSIZE 1024
#define MBSIZE 1048576
#define GBSIZE 1073741824
#define TBSIZE 1099511627776
#define SIZES

#endif /* ! SIZES */
#ifndef DEFAULT_BLK_SIZE
#define DEFAULT_BLK_SIZE 512
#endif /* !DEFAULT_BLK_SIZE */

#define MIN_BLK_SIZE DEFAULT_BLK_SIZE
#define MAX_BLK_SIZE GBSIZE


extern int block_size;
extern UsrOptions * usr_opt;
extern char * LS_PATH;


/*
 * Print the actual relative path
 * By getting the actual pwd, printing only the part that differs from
 * the ls absolute path
 *
 * TODO: When the working directory is really really long getcwd fails
 */
void
PrintTargetPath(char * target) {
    /* Craft path relative to the actual target */
    char * actual_path;
    char * ls_path_char;
    char * actual_path_char;

    actual_path = getcwd(NULL, 0); 
    ls_path_char = LS_PATH;
    actual_path_char = actual_path;

    while (*ls_path_char == *actual_path_char) {
	 ls_path_char++;
	 actual_path_char++;
    }
    
    if (*actual_path_char == '/')
        actual_path_char++;

    printf("\n%s/%s:\n", actual_path_char, target);
    free(actual_path);

}

/*
 * Note: Normally we would not need to handle
 * multiple directories path as we move and list
 * files dynamically, changing the cwd. So each time
 * this function will be called, it will be called on 
 * pathname that directly represent a file.
 */
int 
IsStartingWithDot(char * pathname)
{
    return (pathname[0] == '.');
}

/*
 * return EXIT_FAILURE (0) if the path is "." or ".."
 * return exit_SUCCESS (1) if the path is not "." or ".."
 *
 * Note: Since we can't create hardlink to directories
 * and symlink is considered another file than . or ..
 * Checking only the string should be sufficient
 */ 
int 
IsDotDirectory(char * pathname)
{
    int len = strlen(pathname);

    if (pathname[len-1] == '/') {
        pathname[len-1] = '\0';
    }

    if (strcmp(pathname, "..") == 0
		    || strcmp(pathname, ".") == 0) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;

}

/*
 * return 0 if the path is a non hidden file
 * return 1 if the path is a hidden file
 */
int 
IsHidden(char * pathname)
{
    int len = strlen(pathname);
    int recover = 0;
    int ret;
    char * end;

    /* 
     * We need checks on the string to be done by strcmp and not strncmp
     * To properly discriminates hidden files from .. or .
     * as ..a* or .a* could be considered as .. and . by strncmp
     */ 
    if (strcmp(pathname, "..") == 0 || strcmp(pathname, ".") == 0)
        return 0;

    /* If we have a trailing '/' remove it and replace it later */
    if (pathname[len-1] == '/') 
    {
        pathname[len-1] = '\0';
        recover++;
    }

    /* 
     * search for the trailing chunk that is the path target 
     * If found place the cursor one char after it
     * else just use the start of the pathname
     */
    end = strrchr(pathname, '/');

    if (end == NULL)
        end = pathname;
    else
        end++;

    /* If the end is .. or . then it's not a hidden file */
    if (!strcmp(end, "..") || !strcmp(end, "."))
        ret = 0;
   
    /* else we have a filename so check it's first char is '.' */	
    if (end[0] == '.')
        ret = 1;
    else
        ret = 0;

    /* If you need to place back a trailing '/' do it */
    if (recover)
        pathname[len-1] = '/';

    return ret;
}

int 
CompareMetrics(int metric1, int metric2)
{
    if (metric1 > metric2)
        return -1;
    if (metric1 < metric2)
        return 1;
    else
        return 0;
}

int 
CompareTimeMetrics(struct timespec t1, struct timespec t2)
{
    if (t1.tv_sec != t2.tv_sec)
    {
        if (t1.tv_sec > t2.tv_sec)
            return -1;
        else
            return 1;
    }
    else
    {
        if (t1.tv_nsec > t2.tv_nsec)
            return -1;
        if (t1.tv_nsec < t2.tv_nsec)
            return 1;
        else
            return 0;
    }
}


void 
Padding(char * str1, int longest)
{
     
    int spaces = longest - strlen(str1);

    while (spaces > 0)
    {
        printf(" ");
        spaces--;
    }
}

int 
NbDigitFromInt(int val)
{
    int nb_digit = 0;

    while (val > 0)
    {
        nb_digit++;
	val /= 10;
    }

    return nb_digit;
}

/* 
 * Function to handle BLOCKSIZE given in this format
 * 1b
 * 125k
 * 12m
 * ...
 *
 * It will ensure that the var is valid as well as returning a multiplicator
 * that will be used in further computations
 *
 * basically doing the job of strtol that will consider that
 * 10523kl52 is a valid integer
 */
static long int 
CheckForMult(const char * var)
{
    long int mult = 1;

    /* skip everything until first char that is alphabetic */
    while (*var >= 48 && *var <= 57 )
        var++;

    /* It's a number so return mult of 1*/
    if (var == NULL)
        return mult;

    /* We found a potential trailing alphabetic char */
    switch(*var)
    {
	case 'k':
	    mult = KBSIZE;
	    break;
	case 'm':
	    mult = MBSIZE;
	    break;
	case 't':
	    mult = TBSIZE;
	    break;
	default:
	    mult = -1;
	    break;
    }

    /* If the trailing alphabetic char is invalid return -1 */
    if (mult == -1)
        return mult;

    /* Else check that we do not have any chars after the trailing alpha char*/
    var++;

    if (*var != '\0')
        return -1;
    else
	return mult;
  }

static long 
IsValidInt(const char * var)
{
    char *ep;
    long var_value;
    long int multiplier;

    if (var == NULL)
        return -1;
 
    var_value = strtol((const char *) var, &ep, 10);

    /* var is not a number */
    if (ep == var)
        return -1;

    multiplier = CheckForMult(var);

    if (multiplier < 0)
	    return -1;
    else
        var_value *= multiplier;


    return var_value;
}

/* 
 * Retreive the BLOCKSIZE environment variable 
 * Check if the variable is correct
 * On errors return the default size (512B) with proper error message
 * On success return the blocksize extract from the variable
 */
int 
GetBlockSize(void)
{
    const char * val = getenv("BLOCKSIZE");
    int value;

    if (val == NULL)
        return DEFAULT_BLK_SIZE;

    value = IsValidInt(val);

    if (value == -1)
    {
	if (val)
	    printf("ls: %s: unknown blocksize\n", val);
	else
	    printf("ls: failed to read BLOCKSIZE env variable\n");
	
	printf("ls: maximum blocksize is 1G\n");

	if (val)
	    printf("ls: %s: minimum blocksize is 512\n", val);
	else
	    printf("ls: minimum blocksize is 512\n");	
	
        return DEFAULT_BLK_SIZE;
    }
    else
    {
        if (value < MIN_BLK_SIZE)
	{
	    printf("ls: %s: minimum blocksize is 512\n", val);
	    return DEFAULT_BLK_SIZE;
	}
	else if (value > MAX_BLK_SIZE)
	{
	    printf("ls: maximum blocksize is 1G\n");
	    return DEFAULT_BLK_SIZE;
	}
	else
	    return value;
    }
}


/* 
 * From a number of given blocks of 512B, Convert it in unit of
 * BLOCKSIZE, if BLOCKSIZE was not defined for any reasons
 * It falls back to 512B blocks unit
 */
int 
ComputeBlock(int nb_blocks)
{
    int nb_bytes;

    if (nb_blocks == 0)
        return 0;

    nb_bytes = nb_blocks * 512;
       
    if (usr_opt->k)
    {
	    /* Divide by 1024 to get KB, override any user given block size */
	    return (nb_bytes + KBSIZE - 1) / KBSIZE;
    }    

    return (nb_bytes + block_size - 1) / block_size;
}


/*
 * From nb_bytes bytes, print and return the number of bytes
 * in the most apropriate unit.
 */
long double 
ComputeBytes(long double nb_bytes)
{
    long int unit = 0;

    if (nb_bytes == 0)
       return 0; 
    
    if (nb_bytes != 0)
    {
        if (nb_bytes < KBSIZE)
	    unit = BSIZE;
	else if (nb_bytes >= KBSIZE && nb_bytes < MBSIZE)
	    unit = KBSIZE;
	else if (nb_bytes >= MBSIZE && nb_bytes < GBSIZE)
	    unit = MBSIZE;
	else if (nb_bytes >= GBSIZE && nb_bytes < TBSIZE)
	    unit = GBSIZE;
	else if (nb_bytes >= TBSIZE)
	    unit = TBSIZE;
    }

    return nb_bytes / unit;
}

/* 
 * Function to decide when to print the padding for 
 * Classical printer
 */ 
int 
CheckPaddingWithStep(FileList * list, int step)
{
    if (list)
    {
        while (step)
	{
	    if (list->next == NULL)
	        return 0;
	    list = list->next;
	    step--;
	}
    }
    return 1;
}
