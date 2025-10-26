#include <stdio.h>
#include <stdlib.h>

#include "cd.h"
#include "echo.h"

int main(void)
{
	printf("Welcome to sish_main\n");
	cd_main();
	echo_main();	
	return EXIT_SUCCESS;
}
