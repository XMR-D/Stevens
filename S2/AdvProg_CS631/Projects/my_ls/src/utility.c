#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utility.h"

int IsHidden(char * pathname)
{
    int len = strlen(pathname);
    int recover = 0;
    int ret;

    if (strcmp(pathname, "..") || strcmp(pathname, "."))
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
        end += 1;

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
        return 1;
    if (metric1 < metric2)
        return -1;
    else
        return 0;
}

int CompareTimeMetrics(struct timespec t1, struct timespec t2)
{
    if (t1.tv_sec != t2.tv_sec)
    {
        if (t1.tv_sec > t2.tv_sec)
            return 1;
        else
            return -1;
    }
    else
    {
        if (t1.tv_nsec > t2.tv_nsec)
            return 1;
        if (t1.tv_nsec < t2.tv_nsec)
            return -1;
        else
            return 0;
    }
}
