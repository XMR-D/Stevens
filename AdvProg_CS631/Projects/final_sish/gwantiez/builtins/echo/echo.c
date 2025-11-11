#include "echo.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * expand_var: Routine that print the value of the evironement
 * variable named env_var.
 *
 * Note: None
 */
static int
expand_var(char *env_var)
{
        int shift = 0;
        char *ptr = env_var;
        char saved;

        while (*ptr != '\0' && *ptr != '.' && *ptr != '$') {
                ptr++;
                shift++;
        }

        saved = *ptr;
        *ptr = '\0';
        shift++;

        char *var = getenv(env_var);

        if (var != NULL) {
                printf("%s", var);
        }

        *ptr = saved;
        return shift;
}

/*
 * expand_word: Routine called when a '$' is found,
 * it will try to expand the word specified directly after.
 *
 * if it's '$' : print the current pid
 *
 * if it's '?' : print last exit status
 *
 * if it's '\0': print '$' as a symbol
 *
 * if it's any other char that mark the start of a word:
 * print the env variable named after it
 *
 * Note: None
 */
static int
expand_word(char *word, int last_exit_status)
{
        int shift = 0;
        word = word + 1;
        switch (*word) {
        case '$':
                // print current PID
                printf("%d", getpid());
                shift = 2;
                break;
        case '?':
                // print last exit status
                printf("%d", last_exit_status);
                shift = 2;
                break;
        case '\0':
                // print $ as a char
                printf("$");
                shift = 1;
                break;
        default:
                // else print word as an env variable
                shift = expand_var(word);
                break;
        }
        return shift;
}

static int
print_echo_word(char *input, int last_exit_status)
{
        int shift;
        while (*input != '\0') {
                if (*input == '$') {
                        shift = expand_word(input, last_exit_status);
                } else {
                        printf("%c", *input);
                        shift = 1;
                }
                input += shift;
                shift = 0;
        }
        return EXIT_SUCCESS;
}

int
echo_main(int argc, char **argv, int last_exit_status)
{
        if (argv == NULL) {
                return EXIT_SUCCESS;
        }

        for (int i = 1; i < argc; i++) {
                print_echo_word(argv[i], last_exit_status);
                if (i != (argc - 1)) {
                        printf(" ");
                }
        }
        printf("\n");

        return EXIT_SUCCESS;
}
