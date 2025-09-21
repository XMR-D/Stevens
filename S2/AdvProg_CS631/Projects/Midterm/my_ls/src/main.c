#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ls.h"

int main(int argc, char ** argv)
{
    return wrapper_ls(argc, argv);	
}
