#include <stdio.h>
#include <stdlib.h>

#include "echo.h"

//Print variable as an environment variable
static int
expand_var(char * env_var)
{
	printf("env_var requested : %s", env_var);
	return EXIT_SUCCESS;
}

static int 
expand_input(char * input)
{
	char * ptr = input + 1;
	char save = 0;

	while (*ptr != '\0' || *ptr != ' ' || *ptr == '$') {
		ptr++;
	}
	
	switch(*ptr) {
		case '$':
			//print current PID
			break;
		case '?':
			//print exit status
			break;
		default:
			save = *ptr;
			*ptr = '\0';
			expand_var(input);
			*ptr = save;
			break;
	}

	return EXIT_SUCCESS;

}

int 
echo_main(char * input)
{
	char * ptr = input;
	int printed = 0;

	while (ptr != NULL) {

		if (*ptr == '$') {
			expand_input(ptr);
		} 
		if (*ptr != ' ') {
			if (printed) {
				printf(" ");
			}
			printf("%c", *ptr);
			printed++;
		}
		ptr++;
	}
	printf("\n");
	return EXIT_SUCCESS;
}
