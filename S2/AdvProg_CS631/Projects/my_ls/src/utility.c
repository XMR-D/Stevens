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
char * FullName(char * dirname, char * filename)
{
    int f_len = strlen(filename);
    int d_len = strlen(dirname);

    if (dirname[d_len-1] == '/')
        dirname[d_len-1] = '\0';

    char * filecpy = calloc(sizeof(char), f_len + 1);
    char * dircpy = calloc(sizeof(char), d_len + 1);

    strcpy(dircpy, dirname);
    strcpy(filecpy, filename);

    char * fullname = calloc(sizeof(char), f_len + d_len + 2);

    strcat(fullname, dirname);
    strcat(fullname, "/");
    strcat(fullname, filename);

    free(filecpy);
    free(dircpy);

    return fullname;
}
