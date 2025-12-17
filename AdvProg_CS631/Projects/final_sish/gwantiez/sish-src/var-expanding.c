#include "portability.h"

#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

/* 
 * data can be at most INT_MAX len which is
 * 12 characters long
 * It's high enough to be sure each systems
 * can represent the max_pid
 * On most modern linux the max value is by default
 * 4194304 on Netbsd and Omnios, it's either 30000
 * or 99999
 */
#define MAX_PID_SIZE 12

static char *
concat_char(char* prev_res, char c)
{
	int len = strlen(prev_res) + 2;
	char * res = calloc(1, len);
	if (res == NULL) {
		free(prev_res);
		return NULL;
	}

	char char_str[2] = {0};
	char_str[0] = c;
	
	strcat(res, prev_res);
	strcat(res, char_str);
	free(prev_res);
	return res;
}


static char *
concat_data(char* prev_res, char* data)
{
	int len = strlen(prev_res) + strlen(data) + 1;
	char * res = calloc(1, len);
	if (res == NULL) {
		free(prev_res);
		free(data);
		return NULL;
	}
	
	strcat(res, prev_res);
	strcat(res, data);
	free(data);
	free(prev_res);
	return res;
}


/*
 * expand_var: Routine that print the value of the evironement
 * variable named env_var.
 *
 * Note: None
 */
static int
expand_var(char *env_var, char ** res)
{
        int shift = 0;
        char* ptr = env_var;
        char saved;
	char* var;
	char* dup_var;

        while (*ptr != '\0' && *ptr != '.' && *ptr != '$') {
                ptr++;
                shift++;
        }

        saved = *ptr;
        *ptr = '\0';
        shift++;

        var = getenv(env_var);
	
	if (var != NULL) {
		dup_var = strdup(var);
	}

        if (var != NULL) {
		*res = concat_data(*res, dup_var);
		if (*res == NULL) {
			return -1;
		}
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
expand_word(char *word, char ** prev_res, int last_exit_status, int last_background_pid)
{
        int shift = 0;
	char * data = NULL;
	int do_concat = 1;
	int datalen = MAX_PID_SIZE;
	char * res = *prev_res;
        	
        word = word + 1;
	
	data = calloc(1, datalen);
	if (data == NULL) {
		return -1;
	}

	switch (*word) {
        case '$':
                /* concat the current PID */
                snprintf(data, datalen, "%d", getpid());
                shift = 2;
                break;
        case '?':
                /* concat last exit status */
                snprintf(data, datalen, "%d", last_exit_status);
                shift = 2;
                break;
	case '!':
		/* concat last background pid */
		if (last_background_pid != -1) {
			snprintf(data, datalen, "%d", last_background_pid);
		}
		shift = 2;
		break;
        case '\0':
                /* concat $ as a char */
                data[0] = '$';
                shift = 1;
                break;
        default:
                /* concat the envvalue of word env */
		shift = expand_var(word, prev_res);
		do_concat = 0;
		break;
        }

	if (do_concat) {
		*prev_res = concat_data(res, data);
	}	
	if (!res) {
		return -1;
	}

	if (data != NULL) {
		free(data);
	}
        return shift;
}

char *
expand_tok(char *input, int last_exit_status, int last_background_pid)
{
        int shift;
	char * ret = calloc(1, 1);
	if (ret == NULL) {
		return NULL;
	}
	strcpy(ret, "");

        while (*input != '\0') {
                if (*input == '$') {
			shift = expand_word(input, &ret, last_exit_status, 
					last_background_pid);
			if (shift == -1) {
				free(ret);
				return NULL;
			}
                } else {
			ret = concat_char(ret, *input);
                        shift = 1;
                }
                input += shift;
                shift = 0;
        }
        return ret;
}
