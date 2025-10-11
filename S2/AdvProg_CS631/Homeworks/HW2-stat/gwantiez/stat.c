/*
 * Copyright (c) 2025 Guillaume Wantiez (alias XMR)
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

/*
 * Simple stat implementation with -F flag behavior
 * Display file status information with type indicators
 */

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Function prototypes */
static int print_file_info(const char *);
static int print_mode(struct stat *);
static int print_time(struct stat *);
static char get_file_type_indicator(mode_t);
static void print_indicator(struct stat *, const char *);
static int get_nb_digit(int);
int main(int, char *[]);

/* 
 * Define the maximum date length (without the year)
 * as 15 chars + one terminating null byte
 *
 * i.e. Sep 22 21:30:37
 * 	3 + 1 + 2 + 1 + 8 + 1 (\0) = 16
 */
#define MAX_DATE_LEN 16

/* 
 * Define the maximum mode field length as 10 chars 
 * + one terminating null byte
 *
 * i.e. -rwxrwxrwx
 * 	10 + (\0) = 11
 */
#define MAX_MODE_LEN 11

int
main(int argc, char *argv[])
{
	int F_flag = 0;
	char opt;

	while ((opt = getopt(argc, argv, "F")) != -1) {
		switch (opt) {
		case 'F':
			F_flag = 1;
			break;
		default:
			break;
		}
	}

	if (F_flag == 0) {
		fprintf(stderr, "stat: error: -F flag not specified.\n");
		return EXIT_FAILURE;
	}

	/* If no arguments other than -F, process stdin */
	if (argc == 2) {
		if (print_file_info("(stdin)")) {
			return errno;
		}
	}

	/* 
	 * Process each argument 
	 * (skip options by skipping all strings starting with '-') 
	 */
	else {
		for (int i = 1; i < argc; i++) {
			if (argv[i][0] != '-') {
				if (print_file_info(argv[i])) {
					return errno;
				}
			}
		}
	}
	return EXIT_SUCCESS;
}

/* 
 * This routine print information about a file named filename
 * If an error is encountered, set retcode to errno
 * 
 * Note: None
 */
static int
print_file_info(const char *filename)
{
	struct stat sb;
	struct passwd *pw;
	struct group *gr;
	int result;

	/* Special case for stdin, it's len is 7*/
	if (strncmp(filename, "(stdin)", 7) == 0) {
		result = fstat(STDIN_FILENO, &sb);
	} else {
		result = lstat(filename, &sb);
	}

	if (result == -1) {
		fprintf(stderr, "stat: error: %s: %s\n", filename, strerror(errno));
		return errno;
	}

	/* Print permissions */
	if(print_mode(&sb)) {
		fprintf(stderr, "stat: error: %s: %s\n", filename, strerror(errno));
		return errno;
	}

	/* Print links */
	printf("%lu", (unsigned long)sb.st_nlink);

	/* Print user and group */
	pw = getpwuid(sb.st_uid);
	if (pw == NULL) {
		printf(" (%i)", sb.st_uid);
	} else {
		printf(" %s", (pw != NULL) ? pw->pw_name : "UNKNOWN");
	}
	
	gr = getgrgid(sb.st_gid);
	if (gr == NULL) {
		printf(" (%i)", sb.st_gid);	
	} else {
		printf(" %s", (gr != NULL) ? gr->gr_name : "UNKNOWN");
	}


	/* Print size */
	if (S_ISCHR(sb.st_mode) || S_ISBLK(sb.st_mode)) {
    		printf(" %d,%d ", major(sb.st_rdev), minor(sb.st_rdev));
	} else {
    		printf(" %ld ", (long)sb.st_size);
	}

	/* Print time */
	if(print_time(&sb) != 0) {
		return errno;
	}

	/* Print filename with indicator */
	printf(" %s", filename);
	print_indicator(&sb, filename);
	
	printf("\n");
	return EXIT_SUCCESS;
}


/* 
 * This routine print the character indicating the type of the file
 * it take a stat struct representing the file filename informations
 * and return voiding after the print
 *
 * Note: None
 */
static void
print_indicator(struct stat * sb, const char * filename)
{
	char indicator;
	char linkpath[PATH_MAX] = {0};
	int linklen;

	indicator = get_file_type_indicator(sb->st_mode);
	if (indicator != ' ') {
		printf("%c", indicator);
	}
	if (S_ISLNK(sb->st_mode)) {
    		linklen = readlink(filename, linkpath, sizeof(linkpath) - 1);
    		if (linklen != -1) {
        		linkpath[linklen] = '\0';  
       			printf(" -> %s", linkpath);
    		} else 
			printf(" -> [invalid]");
	}

}


/* 
 * This routine print the mode of the file in a pretty formated way
 * it take a stat struct representing a file as parameter and
 * return EXIT_SUCCESS or errno in case an error has been encountered
 *
 * Note: None
 */
static int
print_mode(struct stat * sb)
{
	char * res = calloc(sizeof(char), MAX_MODE_LEN);
	if (res == NULL) {
		fprintf(stderr, "\nstat: %s\n", strerror(errno));
		return errno;
	}

	strmode(sb->st_mode, res);
	printf("%s", res);
	free(res);
	return EXIT_SUCCESS;
}


/* 
 * This routine print the last modification date in a human readeable format
 * it take a stat struct representing a file as parameter and
 * return EXIT_SUCCESS or errno in case an error has been encountered
 *
 * Note: None
 */
static int
print_time(struct stat * sb)
{
     	struct tm *info = localtime(&(sb->st_mtime));

	/* the final time string size is MAX_DATE_LEN + the number of digit of the
	 * year the file was created, tm_year is an indicator of number of years
	 * since 1900. (so we need to add 1900 to have the actual date) */
	int buffer_size = MAX_DATE_LEN + get_nb_digit(info->tm_year + 1900);

	char * out_str = calloc(sizeof(char), buffer_size);
	if (out_str == NULL) {
		fprintf(stderr, "stat: error: %s.\n", strerror(errno));
		return errno;
	}

     	strftime(out_str, buffer_size, "%b %e %H:%M:%S %Y", info);
     	printf("%s", out_str);
	free(out_str);
	return EXIT_SUCCESS;
}


/* 
 * This routine will return an appropriate character depending on mode
 * it take a mode_t mode as parameter and return a character.
 * 
 * Note: None
 */
static char
get_file_type_indicator(mode_t mode)
{
	if (S_ISDIR(mode)) {
		return '/';
	}
	if (S_ISLNK(mode)) {
		return '@';
	}
	if (S_ISFIFO(mode)) {
		return '|';
	}
	if (S_ISSOCK(mode)) {
		return '=';
	}
	if (S_ISWHT(mode)) {
		return '%';
	}

	if ((mode & S_IXUSR) || (mode & S_IXGRP) || (mode & S_IXOTH)) {
		return '*';
	}

	return ' ';
}

static int
get_nb_digit(int value)
{
	int count = 0;

	while(value > 0) {
		value /= 10;
		count++;
	}
	return count;
}
