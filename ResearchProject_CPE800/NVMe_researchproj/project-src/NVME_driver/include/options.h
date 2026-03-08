#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdint.h>

typedef struct opt_flgs {
    uint8_t h;
} Opt_flgs;

/*
 * parse_cmd : Routine that will parse the options passed in the
 * command line argument by the user
 *
 * Note: Once a token that is not an option is met, the tokenizing stop
 * 	 Considering each subsequent tokens as targets, even if formated
 * 	 like options
 */
Opt_flgs * parse_options(int *argc, char ** argv[]);
void print_help(void);

#endif /* !OPTIONS_H */