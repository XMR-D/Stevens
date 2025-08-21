#include <stdlib.h>
#include <stdio.h>
#include <string.h> 

#include "log.h"
#include "error.h"
#include "opt_parser.h"

/* usr_opt pointer that will contain the options for later purpose */
UsrOptions * usr_opt;

int OptSet(char * token)
{
    //TODO
    *token = 'a';
    return 0;
}
