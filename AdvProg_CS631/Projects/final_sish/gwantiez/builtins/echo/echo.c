#include "portability.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "echo.h"
int
echo_main(int argc, char **argv)
{
        if (argv == NULL) {
                return EXIT_SUCCESS;
        }

        for (int i = 1; i < argc; i++) {
                printf("%s", argv[i]);
                if (i != (argc - 1)) {
                        printf(" ");
                }
        }
        printf("\n");

        return EXIT_SUCCESS;
}
