#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utility.h"

#ifndef DEFAULT_BLK_SIZE
#define DEFAULT_BLK_SIZE 512
#endif /* !DEFAULT_BLK_SIZE */

#define MIN_BLK_SIZE 512
#define MAX_BLK_SIZE 1073741824 /* 1G size */

int IsHidden(char * pathname)
{
    int len = strlen(pathname);
    int recover = 0;
    int ret;

    if (strcmp(pathname, "..") == 0 || strcmp(pathname, ".") == 0)
        return 0;

    if (pathname[len-1] == '/') 
    {
        pathname[len-1] = '\0';
        recover++;
    }

    char * end = strrchr(pathname, '/');

    if (end == NULL)
        end = pathname;
    else
        end++;

    if (!strcmp(end, ".."))
        ret = 0;
    if (end[0] == '.')
        ret = 1;
    else
        ret = 0;

    if (recover)
        pathname[len-1] = '/';

    return ret;
}

int CompareMetrics(int metric1, int metric2)
{
    if (metric1 > metric2)
        return -1;
    if (metric1 < metric2)
        return 1;
    else
        return 0;
}

int CompareTimeMetrics(struct timespec t1, struct timespec t2)
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


void Padding(char * str1, int longest)
{
     
    int spaces = longest - strlen(str1);

    while (spaces > 0)
    {
        printf(" ");
        spaces--;
    }
    
}

int NbDigit(int val)
{
    int nb_digit = 0;

    while (val > 0)
    {
        nb_digit++;
	val /= 10;
    }

    return nb_digit;
}

static long IsValidInt(const char * var)
{
    char *ep;
    long var_value;

    if (var == NULL)
        return -1;
    
    var_value = strtol((const char *) var, &ep, 10);

    /* var is not a number */
    if (ep == var)
        return -1;

    return var_value;
}

int GetBlockSize(void)
{
    const char * val = getenv("BLOCKSIZE");

    int value = IsValidInt(val);

    if (val == NULL || value == -1)
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
