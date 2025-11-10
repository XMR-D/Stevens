#include "cd.h"

#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

int cd_main(int argc, char **argv)
{
    char *target_path;
    struct passwd *pw;

    
    if (argc > 2) {
        fprintf(stderr, "cd: too many arguments\n");
        return EXIT_FAILURE;
    }
    
    if (argc == 1) {
        target_path = getenv("HOME");
        if (!target_path || target_path[0] == '\0') {
            uid_t uid = getuid();
            pw = getpwuid(uid);
            
            if (!pw || !pw->pw_dir || pw->pw_dir[0] == '\0') {
                fprintf(stderr, "cd: HOME not set\n");
                return EXIT_FAILURE;
            }
            target_path = pw->pw_dir;
        }
    } else {
        target_path = argv[1];
        if (target_path[0] == '\0') {
            fprintf(stderr, "cd: invalid directory\n");
            return EXIT_FAILURE;
        }
    }

    
    if (chdir(target_path) == -1) {
        fprintf(stderr, "cd: error %s: %s\n", target_path, strerror(errno));
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
