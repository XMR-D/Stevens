#include "cd.h"

#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

int cd_main(int argc, char ** argv)
{
	char * target_path;
	struct passwd * pw;
	uid_t uid;

	if (argc > 2) {
		warnx("bad substitution");
		return EXIT_FAILURE;
	}

	if (argc == 1) {
		target_path = getenv("HOME");
		if (!target_path) {

			uid = getuid();
    			pw = getpwuid(uid);

			if (pw == NULL) {
				warnx("cd: HOME variable is unset and"
				      "user home directory could "
				      "not be found");
				return EXIT_FAILURE;
			}

			target_path = pw->pw_dir;
		}
	} else {
		target_path = argv[1];
	}

        if (chdir(target_path) == -1) {

		warnx("cd: error: %s", strerror(errno));
		return errno;
	};

	return EXIT_SUCCESS;
}
