#include <stdlib.h>
#include <stdio.h>
#include <string.h> 

#include "log.h"
#include "error.h"
#include "opt_parser.h"
#include "targ_parser.h"
#include "tokenize.h"

#define ERR_THROWN

/* opt_delim is put to 1 when '--' is met, meaning next tokens are targets */
int opt_delim = 0;

/* small interrupt signal to optimize in case of help query (such as --help) */
int help = 0;

int process_token(char * token)
{
    /* token is an help request*/
    if (!strcmp(token, "--help"))
    {
        FUNC_LOG("process_token", "HELP found");
        help++;
        return 0;
    }
    /* token is an option*/
    if (token[0] == '-' && !opt_delim)
    {
        //treat token as option
        FUNC_LOG("process_token", "OPTION found");

        if (OptSet(token))
            return WRNG_OPT_ERR;
        else
            return 0;

    }
    /* If the token '--' then state that delim has been found */
    if (!strcmp(token, "--") && !opt_delim)
    {
        FUNC_LOG("process_token", "DELIM found");
        opt_delim++;
        return 0;
    }
    /* token is a target */
    else
    {
        FUNC_LOG("process_token", "TARGET found");

        if (TargPush(token))
            return WRNG_TARG_ERR;
        else
            return 0;
    }
}

int tokenize(int argc, char * input[])
{
    WARN("Tokenizer start.");
    int err = 0;

    for (int i = argc; i >= 1 && !help; i--)
    {
        FUNC_LOG("main", "processing token : ");
        printf("%s\n", *input);
        
        err = process_token(*input);
        if (err)
            return err;

        input++;
        printf("\n");
    }
    SUCCESS("Tokensizer end.");


    //STEP 2 WORLD
    return 0;
}