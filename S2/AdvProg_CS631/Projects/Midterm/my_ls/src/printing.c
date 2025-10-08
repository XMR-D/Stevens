#include "printing.h"

#include <sys/ioctl.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <math.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "error.h"
#include "opt_parser.h"
#include "padding-handling.h"
#include "utility.h"

/* defined in ls.c */
extern UsrOptions *USR_OPT;
extern PaddingInfos *PINFOS;
extern long BLOCKSIZE;

/* defined in listing.c */
extern int PRINTED;

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
 * This macro represent a year in seconds
 * Nb_days_in_a_year * Nb_hours_in_a_day
 * 	* Nb_minutes_in_a_hour * nb_seconds_in_a_minutes
 *
 * 365 * 24 * 60 * 60 = 31536000
 *
 * Used to check if a file has been created long ago.
 */
#define YEAR_IN_SECONDS 31536000

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


#ifndef SIZES
#define SIZES
#define BSIZE 1
#define KBSIZE 1024
#define MBSIZE 1048576
#define GBSIZE 1073741824
#define TBSIZE 1099511627776
#endif /* ! SIZES */

static void
PrintFileName(char *filename)
{
    /* Force printing of non-printable characters as '?' */
    if (USR_OPT->q) {
        while (*filename != '\0') {
            if (*filename < ' ' || *filename > '~') {
                putc('?', stdout);
            } else {
                putc(*filename, stdout);
            }

            filename++;
        }
    }
    /* Force raw printing of non-printable characters */
    else if (USR_OPT->w) {
        fwrite(filename, sizeof(char), strlen(filename), stdout);
    }
    /* Else, print the file name normally */
    else {
        printf("%s", filename);
    }
}

/* If padding_order = 0 the padding is after the value, else it's before */
static int
PrintIntVal(int padding_order, long int val, long int max_len)
{
    /* +1 to account for the '\0' */
    int val_size = NbDigitFromInt(val) + 1;
    char *val_str = calloc(sizeof(char), val_size);
    if (!val_str) {
        throw_error("", MEM_ERR);
        return errno;
    }

    snprintf(val_str, val_size, "%ld", val);

    if (padding_order) {
        print_padding(val_str, max_len);
    }

    printf("%s", val_str);

    if (padding_order == 0) {
        print_padding(val_str, max_len);
    }

    free(val_str);
    return EXIT_SUCCESS;
}

static int
PrintOwner(struct stat sb)
{
    if (!USR_OPT->n) {
        struct passwd *pwd = getpwuid(sb.st_uid);

        /*
         * If getpwuid failed, fall back on printing the raw number
         * with the max padding computed from the string representation
         * of each file uid
         */
        if (pwd == NULL) {
            if (PrintIntVal(0, sb.st_uid, PINFOS->max_uid_str_len)) {
                return errno;
            }
        } else {
            printf("%s", pwd->pw_name);
            print_padding(pwd->pw_name, PINFOS->max_uid_str_len);
        }
    } else {
        if (PrintIntVal(0, sb.st_uid, PINFOS->max_uid_int_len)) {
            return errno;
        }
    }
    printf("  ");
    return EXIT_SUCCESS;
}

static int
PrintGroup(struct stat sb)
{
    if (!USR_OPT->n) {
        struct group *grp = getgrgid(sb.st_gid);

        /*
         * If getgrgid failed, fall back on printing the raw number
         * with the max padding computed from the string representation
         * of each file gid
         */
        if (grp == NULL) {
            if (PrintIntVal(0, sb.st_gid, PINFOS->max_gid_str_len)) {
                return errno;
            }
        }

        else {
            printf("%s", grp->gr_name);
            print_padding(grp->gr_name, PINFOS->max_gid_str_len);
        }
    } else {
        if (PrintIntVal(0, sb.st_gid, PINFOS->max_gid_int_len)) {
            return errno;
        }
    }

    printf("  ");
    return EXIT_SUCCESS;
}

static void
PrintDate(struct stat sb)
{
    char out_str[MAX_DATE_LEN] = {'\0'};
    struct tm *info;
    time_t now;
    char *date_to_print;
    int time_err;
    int time_diff;

    /* 
     * tzset will retreive TZ environement value and set it automatically
     * see tzset(3) for more information
     */
    tzset();
    info = localtime(&(sb.st_mtime));
    time_err = time(&now);
    time_diff = difftime(now, sb.st_mtime);


    /* 
     * If the time difference between now and the file creation is 
     * more than a year ago, print only the date
     */
    if ((time_err == -1) || abs((int)time_diff) < YEAR_IN_SECONDS) {
        date_to_print = "%b %e %H:%M";
    } else {
        date_to_print = "%b %e  %Y";
    }

    strftime(out_str, sizeof(out_str), date_to_print, info);
    printf("%s", out_str);
}

static void
PrintHumanReadable(int byte_nb, int padding_size)
{

    char pbuf[MAX_BYTE_FIELD_LEN] = {0};
    humanize_number(pbuf, MAX_BYTE_FIELD_LEN, byte_nb, 0, HN_AUTOSCALE,
                    HN_B | HN_DECIMAL | HN_NOSPACE);
    print_padding(pbuf, padding_size);
    printf("%s", pbuf);
}

static void
PrintTotalByte(void)
{
    printf("total ");
    if (USR_OPT->h) {
        PrintHumanReadable(PINFOS->total_bytes, 0);
    } else {
        int nb_byte = (PINFOS->total_blocks * BLOCKSIZE);
        printf("%d", (int)round(nb_byte / BLOCKSIZE));
    }
    printf("\n");
}

static void
PrintSymlink(FTSENT *parentdir, char *filename)
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

/* Add a character representing the file type after the file name */
static void
Handle_F_Option(struct stat sb)
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
            printf("%c", '%');
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

static int
Handle_i_Option(struct stat sb)
{
    if (USR_OPT->i) {
        if (PrintIntVal(1, sb.st_ino, PINFOS->max_inode_nb_len)) {
            return errno;
        }
        printf(" ");
    }
    return EXIT_SUCCESS;
}

static int
Handle_s_Option(struct stat sb, int fromblocks)
{
    if (USR_OPT->s) {
        if (!USR_OPT->h) {
            if (PrintIntVal(1, ComputeBlock(sb.st_blocks),
                            PINFOS->max_nb_block_len)) {
                return errno;
            }
        } else {
            long double raw_bytes = 0;
            if (fromblocks) {
                raw_bytes = sb.st_blocks * 512;
            } else {
                raw_bytes = sb.st_size;
            }

            /*
             * -1 is used to account for the extra '\0'
             * that impact the computations
             */
            PrintHumanReadable(raw_bytes, MAX_BYTE_FIELD_LEN - 1);
        }
        printf(" ");
    }
    return EXIT_SUCCESS;
}

static int
Handle_l_Option(struct stat sb)
{
    if (USR_OPT->l) {
        /* Print the file mode */
        char res[MAX_MODE_LEN] = {'\0'};
        int padding_size;

        strmode(sb.st_mode, res);
        printf("%s ", res);

        /* Print the number of links */
        if (PrintIntVal(1, sb.st_nlink, PINFOS->max_link_nb_len)) {
            return errno;
        }

        printf(" ");

        PrintOwner(sb);
        PrintGroup(sb);

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
            PrintHumanReadable(sb.st_size, padding_size);
        } else if (PrintIntVal(1, sb.st_size, PINFOS->max_nb_byte_len)) {
            return errno;
        }

        printf(" ");

        /* Print the date */
        PrintDate(sb);
        printf(" ");
    }
    return EXIT_SUCCESS;
}

static int
DispFile(struct stat file_sb, char *filename, FTSENT *parentdir)
{

    if (Handle_i_Option(file_sb)) {
        return errno;
    }

    if (Handle_s_Option(file_sb, 1)) {
        return errno;
    }

    if (Handle_l_Option(file_sb)) {
        return errno;
    }

    /* Print the file name */
    PrintFileName(filename);

    Handle_F_Option(file_sb);

    if ((USR_OPT->l || USR_OPT->F || USR_OPT->d) && S_ISLNK(file_sb.st_mode)) {
        PrintSymlink(parentdir, filename);
    }

    printf("\n");

    /* Reset padding for further prints */
    return EXIT_SUCCESS;
}

/* Call DispFile on every file after computing padding necessary */
int
LongFormatPrinter(FTSENT *parentdir, FTSENT *list)
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
            fprintf(stderr, "ls: %s: %s\n", saved->fts_name,
                    strerror(saved->fts_errno));
            errno = saved->fts_errno;
            saved = saved->fts_link;
            continue;
        }

        compute_padding_needed(*(saved->fts_statp), USR_OPT);
        saved = saved->fts_link;
    }

    /*
     * If the -l or the -s option is set and that we are not printing
     * the command line files then print the total number of bytes
     *
     * TODO: Print the total if the output is to a terminal only !
     */
    if (parentdir != NULL && (USR_OPT->l || USR_OPT->s)) {
        PrintTotalByte();
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
            DispFile(*(list->fts_statp), list->fts_name, parentdir)) {
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
