#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utility.h"

int IsHidden(char * pathname)
{
    int len = strlen(pathname);
    int recover = 0;
    int ret;

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
    else if (end[0] == '.')
        ret = 1;
    else
        ret = 0;

    if (recover)
        pathname[len-1] = '/';

    return ret;
}