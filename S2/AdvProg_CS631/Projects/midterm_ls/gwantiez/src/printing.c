#include "printing.h"

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <math.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "opt_parser.h"
#include "padding-handling.h"
#include "utility.h"

/* defined (and expalined) in ls.c */
extern UsrOptions *USR_OPT;
extern PaddingInfos *PINFOS;
extern long BLOCKSIZE;

/* defined (and explained) in listing.c */
extern int PRINTED;

/* defined (and explained) in opt-parser.c */
extern int PRINT_TOTAL;

/*
 * This macro represent the maximum possible len for a
 * Date formated by ls
 * ex "Dec 31 00:00" is 12 char long + the '\0' = 13
 * it can't be anything else as we do not print year
 * (unless if it's a more than a year later but again 5 chars will be changed to
 * 4 max) which is the only field that can possibly grow the string outside his
 * bounds year 9999->10000
 */
#define MAX_DATE_LEN 13

/*
 * Define the maximum mode field length as 10 chars
 * + one terminating null byte
 *
 * i.e. -rwxrwxrwx
 * 	10 + (\0) = 11
 */
#define MAX_MODE_LEN 11

/*
 * Define the maximum number of char that a
 * human byte representation can take
 * 3 char + 1 unit + 1 terminating null byte
 *
 * i.e. 999M
 * 	3 + 1 + (\0) = 5
 */
#define MAX_BYTE_FIELD_LEN 5

/* print_file_name : This routine will print filename depending
 * on the option passed in USR_OPT for the non-printable char handling
 *
 * Note: (None)
 */
static void
print_file_name(char *filename)
{
    /* If -q force printing of non-printable characters as '?' */
    if (USR_OPT->q) {
        while (*filename != '\0') {
            if (isprint((int)*filename) == 0) {
                putc('?', stdout);
            } else {
                putc(*filename, stdout);
            }

            filename++;
        }
    }
    /* If -w force raw printing of non-printable characters */
    else if (USR_OPT->w) {
        fwrite(filename, sizeof(char), strlen(filename), stdout);
    }
    /* Else, print the file name normally */
    else {
        printf("%s", filename);
    }
}

/*
 * print_int_value : This routine will print val inserting a padding
 * made of ' ' characters computed with max_len.
 *
 * Note : The padding can be inserted before of after the value depending
 * of the need.
 *
 * if padding_order is 1, then the padding will be inserted before the value.
 * if padding_order is 0, then the padding will be inserted after the value.
 * if padding_order is an other value, then no padding is inserted.
 *
 * max_len correspond to the total number of chars on which the value and the
 * padding will be written on, if this value is previously wrong computed then
 * the result is undefined.
 *
 *
 * the routine will return 0 on success, a value >1 if failed.
 */
static int
print_int_value(int padding_order, long int val, long int max_len)
{
    /* +1 to account for the '\0' */
    int val_size = NbDigitFromInt(val) + 1;
    char *val_str = calloc(sizeof(char), val_size);
    if (!val_str) {
	/* free all alloced structures and variables in usage at that point*/
	free(val_str);
	free(USR_OPT);
	free(PINFOS);
	/* will exit the program if called, no return needed */
        errx(1, "ls: memory error: %s\n", strerror(errno));
    }

    snprintf(val_str, val_size, "%ld", val);

    if (padding_order) {
        print_padding(val_str, max_len);
    }

    printf("%s", val_str);

    if (padding_order == 0) {
        print_padding(val_str, max_len);
    }
        return errno;

    free(val_str);
    return EXIT_SUCCESS;
}

/*
 * print_owner : This routine will print the owner informations extracted
 * from sb stat structure depending on the options passed in USR_OPT structure.
 *
 * the routine will return 0 on success, a value >1 if failed.
 *
 * Note : If the user want to print the owner id as a string representation
 * and the conversion fail, then print_owner will default to its integer
 * representation.
 */
static int
print_owner(struct stat sb)
{
    if (!USR_OPT->n) {
        struct passwd *pwd = getpwuid(sb.st_uid);

        /*
         * If getpwuid failed, fall back on printing the raw number
         * with the max padding computed from the string representation
         * of each file uid
         */
        if (pwd == NULL) {
            if (print_int_value(0, sb.st_uid, PINFOS->max_uid_str_len)) {
                return errno;
            }
        } else {
            printf("%s", pwd->pw_name);
            print_padding(pwd->pw_name, PINFOS->max_uid_str_len);
        }
    } else {
        if (print_int_value(0, sb.st_uid, PINFOS->max_uid_int_len)) {
            return errno;
        }
    }
    printf("  ");
    return EXIT_SUCCESS;
}

/*
 * print_group : This routine will print the group informations extracted
 * from sb stat structure depending on the options passed in USR_OPT structure.
 *
 * the routine will return 0 on success, a value >1 if failed.
 *
 * Note : If the user want to print the group id as a string representation
 * and the conversion fail, then print_group will default to its integer
 * representation.
 */
static int
print_group(struct stat sb)
{
    if (!USR_OPT->n) {
        struct group *grp = getgrgid(sb.st_gid);

        /*
         * If getgrgid failed, fall back on printing the raw number
         * with the max padding computed from the string representation
         * of each file gid
         */
        if (grp == NULL) {
            if (print_int_value(0, sb.st_gid, PINFOS->max_gid_str_len)) {
                return errno;
            }
        }

        else {
            printf("%s", grp->gr_name);
            print_padding(grp->gr_name, PINFOS->max_gid_str_len);
        }
    } else {
        if (print_int_value(0, sb.st_gid, PINFOS->max_gid_int_len)) {
            return errno;
        }
    }

    printf("  ");
    return EXIT_SUCCESS;
}

/*
 * print_date : This routine will print the date extracted
 * from sb stat structure depending on the TZ environment variable.
 *
 * Note : The timezone environement variable is automatically set using
 * tzset(3) inside the call of localtime(3) and will handle possible
 * invalid timezone passed by the user.
 *
 * see tzset(3) and localtime(3) man page for more information.
 */
static void
print_date(struct stat sb)
{
    char out_str[MAX_DATE_LEN] = {'\0'};
    struct tm *info;
    char *date_to_print;
    time_t timemetric;

    if (USR_OPT->t && USR_OPT->c) {
	    timemetric = sb.st_ctime;
    } else if (USR_OPT->t && USR_OPT->u) {
	    timemetric = sb.st_atime;
    } else {
	    timemetric = sb.st_mtime;
    }


    info = localtime(&(timemetric));

    date_to_print = "%b %e %H:%M";

    strftime(out_str, sizeof(out_str), date_to_print, info);
    printf("%s", out_str);
}

/*
 * print_human_readable : This routine is a wrapper to humanize_number(3)
 * that will print byte_nb value in a human readable format inserting
 * padding_size chars of padding
 *
 * Note : (None)
 */
static void
print_human_readable(int byte_nb, int padding_size)
{

    char pbuf[MAX_BYTE_FIELD_LEN] = {0};
    humanize_number(pbuf, MAX_BYTE_FIELD_LEN, byte_nb, 0, HN_AUTOSCALE,
                    HN_B | HN_DECIMAL | HN_NOSPACE);
    print_padding(pbuf, padding_size);
    printf("%s", pbuf);
}

/*
 * print_total_byte : This routine will print the total
 * of bytes found in a listing of files in a directory depending
 * on options passed in USR_OPT
 *
 * Note : the total of bytes value is in the PINFOS structure.
 */
static void
print_total_byte(void)
{
    printf("total ");
    if (USR_OPT->h) {
        print_human_readable(PINFOS->total_bytes, 0);
    } else {
        printf("%d", ComputeBlock(PINFOS->total_blocks));
    }
    printf("\n");
}

/*
 * print_symlink : This routine will print the symlink path pointed
 * by filename depending on the parent directory.
 *
 * Note : Symlink resolution is done in the following manner,
 *
 * if parentdir is NULL, it means that filename is a target passed in by the
 * user as a command line argument, so in this case '.' should be used as the
 * CWD to resolve the symlink.
 *
 * if parentdir is not NULL, it means that filename is a file obtained in the
 * fts traversal and parentdir access path should be used to resolve the
 * symlink.
 *
 * Otherwise the path resolution could fail, if it's the case "-> [invalid]"
 * should be printed.
 */
static void
print_symlink(FTSENT *parentdir, char *filename)
{
    char linkpath[PATH_MAX] = {0};
    char *accpath;
    int curr_fd;
    int linklen;

    /*
     * If we are handling a cmd file (no parent directory) just
     * use the current dir to read the link using open.
     * Else use the path from the parent to read the link using open
     */
    if (parentdir == NULL) {
        accpath = ".";
    } else {
        accpath = parentdir->fts_accpath;
    }

    curr_fd = open(accpath, O_RDONLY);
    if (curr_fd == -1) {
        printf(" -> [invalid]");
        return;
    }

    linklen = readlinkat(curr_fd, filename, linkpath, sizeof(linkpath));
    close(curr_fd);

    if (linklen != -1) {
        linkpath[linklen] = '\0';
        printf(" -> %s", linkpath);
    } else {
        printf(" -> [invalid]");
    }
}

/*
 * handle_F_option : This routine will print a trailing character to specify
 * the type of the file listed depending on the -F option in USR_OPT
 *
 * Note : see the section LONGFORMAT of the README.md to get more information
 * on which characters depict which filetype.
 */
static void
handle_F_option(struct stat sb)
{
    if (USR_OPT->F) {
        char filetype;
        char res[MAX_MODE_LEN] = {'\0'};
        strmode(sb.st_mode, res);

        filetype = *res;

        switch (filetype) {
        case 'l':
            printf("@");
            break;
        case 'd':
            printf("/");
            break;
        case 's':
            printf("=");
            break;
        case 'w':
            printf("%%");
            break;
        case 'p':
            printf("|");
            break;
        case '-':
            if (sb.st_mode & S_IXUSR) {
                printf("*");
            }
            break;
        default:
            break;
        }
    }
}

/*
 * handle_i_option : This routine will print the inode number extracted from
 * sb stat structure, depending on the -i option in USR_OPT.
 *
 * the routine will return 0 on success, a value >1 if failed.
 *
 * Note : (None)
 */
static int
handle_i_option(struct stat sb)
{
    if (USR_OPT->i) {
        if (print_int_value(1, sb.st_ino, PINFOS->max_inode_nb_len)) {
            return errno;
        }
        printf(" ");
    }
    return EXIT_SUCCESS;
}

/*
 * handle_s_option : This routine will print the size extracted
 * from sb stat structure depending on the options passed in USR_OPT.
 *
 * Note : This routine will always use the size from the blocks
 * if -h is not specified, and the size from the sizes if -h is specified
 * regardless of if -l is passed or not. That way we are in accordance
 * with the given man page.
 */
static int
handle_s_option(struct stat sb)
{
    if (USR_OPT->s) {
        if (!USR_OPT->h) {
            if (print_int_value(1, ComputeBlock(sb.st_blocks),
                                PINFOS->max_nb_block_len)) {
                return errno;
            }
        } else {
            long double raw_bytes = sb.st_size;
            /* -1 is used to account for the extra '\0' */
            print_human_readable(raw_bytes, MAX_BYTE_FIELD_LEN - 1);
        }
        printf(" ");
    }
    return EXIT_SUCCESS;
}

/*
 * handle_l_option : This routine will print various informations extracted
 * from sb stat structure. This function will follow the guideline in
 * LONGFORMAT part of the README.md to display the informations.
 *
 * the routine will return 0 on success, a value >1 if failed.
 *
 * Note : (None)
 */
static int
handle_l_option(struct stat sb)
{
    if (USR_OPT->l) {
        /* Print the file mode */
        char res[MAX_MODE_LEN] = {'\0'};
        int padding_size;

        strmode(sb.st_mode, res);
        printf("%s ", res);

        /* Print the number of links */
        if (print_int_value(1, sb.st_nlink, PINFOS->max_link_nb_len)) {
            return errno;
        }

        printf(" ");

        print_owner(sb);
        print_group(sb);

        /* Set padding size for number of byte field */
        if (USR_OPT->h) {
            if (PINFOS->max_special_file_byte_len > MAX_BYTE_FIELD_LEN) {
                padding_size = PINFOS->max_special_file_byte_len;
            } else {
                padding_size = MAX_BYTE_FIELD_LEN - 1;
            }
        } else {
            padding_size = PINFOS->max_nb_byte_len;
        }

        /*
         * Handle block special or character special file
         * + 2 to account for the ", " we will print
         */
        if (S_ISCHR(sb.st_mode) || S_ISBLK(sb.st_mode)) {
            int total_len = NbDigitFromInt(major(sb.st_rdev)) +
                            NbDigitFromInt(minor(sb.st_rdev)) + 2;

            print_padding(NULL, padding_size - total_len);
            printf("%d, %d", major(sb.st_rdev), minor(sb.st_rdev));
        }
        /* If h is specified print the number of bytes */
        else if (USR_OPT->h) {
            print_human_readable(sb.st_size, padding_size);
        } else if (print_int_value(1, sb.st_size, PINFOS->max_nb_byte_len)) {
            return errno;
        }

        printf(" ");

        /* Print the date */
        print_date(sb);
        printf(" ");
    }
    return EXIT_SUCCESS;
}

/*
 * disp_file : This routine will print information of filename
 * extracted from it's corresponding file_sb stat structure
 * depending on option passed in USR_OPT.
 *
 * the routine will return 0 on success, a value >1 if failed.
 *
 * Note : parentdir is used to resolve filename if it represent a symlink.
 */
static int
disp_file(struct stat file_sb, char *filename, FTSENT *parentdir)
{
    if (handle_i_option(file_sb)) {
        return errno;
    }

    if (handle_s_option(file_sb)) {
        return errno;
    }

    if (handle_l_option(file_sb)) {
        return errno;
    }

    /* Print the file name */
    print_file_name(filename);

    handle_F_option(file_sb);

    if ((USR_OPT->l || USR_OPT->F || USR_OPT->d) && S_ISLNK(file_sb.st_mode)) {
        print_symlink(parentdir, filename);
    }

    printf("\n");

    /* Reset padding for further prints */
    return EXIT_SUCCESS;
}

/*
 * listing_printer : This routine will call disp_file on all the files
 * in the listing list. parentdir is representing the actual parent directory.
 *
 * the routine will return 0 on success, a value >1 if failed.
 *
 * Note : long_format_printer is the entrypoint of this file and should be the
 * only function callable outside this file.
 */
int
listing_printer(FTSENT *parentdir, FTSENT *list)
{
    FTSENT *saved;

    if (list == NULL) {
        return EXIT_SUCCESS;
    }
    
    saved = list;
    while (saved != NULL) {

        /*
         * If the current file created an error, throw the corresponding error
         * message, set errno, and skip to the next file
         */
        if (saved->fts_errno != 0) { 
            warnx("%s: %s", saved->fts_name, strerror(saved->fts_errno));
            errno = saved->fts_errno;
            saved = saved->fts_link;
            continue;
        }
        compute_padding_needed(*(saved->fts_statp), USR_OPT);
        saved = saved->fts_link;
    }

    /*
     * If PRINT_TOTAL has been set and that we are not printing
     * the command line files then print the total number of bytes
     *
     * (see opt-parser.c for more details on PRINT_TOTAL)
     */
    if (parentdir != NULL && PRINT_TOTAL) {
        print_total_byte();
    }

    while (list != NULL) {

        if (parentdir == NULL && list->fts_info == FTS_D && !USR_OPT->d) {
            list = list->fts_link;
            continue;
        }

        /*
         * If -a is not specified and -A is not specified skip all files
         * that start with '.' . Because "." and ".." are considered
         * directories that start with '.'.
         *
         * if -A is specified, we want to list everything except "." and ".."
         * So we do not want to skip a file that starts with ".".
         * "." and ".." are handled in the fts_flag option, so it should not
         * show up here.
         */
        if (!USR_OPT->a && !USR_OPT->A && list->fts_name[0] == '.') {
            list = list->fts_link;
            continue;
        }
        if (list->fts_errno == 0 &&
            disp_file(*(list->fts_statp), list->fts_name, parentdir)) {
            return errno;
        }
        if (!PRINTED) {
            PRINTED = 1;
        }
	
        list = list->fts_link;
    }

    reset_print_infos(PINFOS);
    return errno;
}
