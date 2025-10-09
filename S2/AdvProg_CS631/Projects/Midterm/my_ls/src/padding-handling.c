#include "padding-handling.h"

#include <sys/stat.h>

#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "opt_parser.h"
#include "utility.h"

/* defined in ls.c */
extern PaddingInfos *PINFOS;

/*
 * print_padding: This routine will print a number of space depending of str
 * and longest
 *
 * longest correspond to an int indicating on which numbers of chars 
 * str must be represented, and that will be used to compute the number of
 * spaces required
 *
 * i.e: if str correspond to the size of a file in blocks and that the longest
 * representation of a block size is 10 in the current countext then:
 *
 * print_padding("150", 10) will print 10 - 4 spaces to align 150
 * to the longest existing in the current context giving as a result:
 * 		
 * 		1234567890
 * 		.......150	< ....... is what print_padding will print.
 *
 * Note: The only thing print_padding is doing is printing a number of spaces
 * printing str need to be handled in printing.c as the padding printing can be 
 * before or after str depending on which field we want to print.
 */
void
print_padding(char *str, int longest)
{
    int spaces;
    int str_len;

    if (str == NULL) {
        str_len = 0;
    } else {
        str_len = strlen(str);
    }

    spaces = longest - str_len;

    while (spaces > 0) {
        printf(" ");
        spaces--;
    }
}

/*
 * reset_print_infos : This routine reset the PaddingInfos structure pointed to 
 * by infos.
 *
 * Note : (None)
 */
void
reset_print_infos(PaddingInfos *infos)
{
    infos->max_link_nb_len = 0;
    infos->max_uid_str_len = 0;
    infos->max_gid_str_len = 0;
    infos->max_uid_int_len = 0;
    infos->max_gid_int_len = 0;
    infos->max_nb_byte_len = 0;
    infos->max_nb_block_len = 0;

    infos->max_inode_nb_len = 0;

    infos->total_bytes = 0;
    infos->total_blocks = 0;
}


static void
max_check(int val, int *max)
{
    if (val > *max) {
        *max = val;
    }
}

/*
 * compute_padding_needed: This routine compute the length of all the 
 * possible numerical fields of the long format printer for all files found.
 * by comparing each field of the file to the pinfos actual structure 
 * that contains the max lengths of the file struct fields.
 *
 * Note: It's a particulary messy function that manually fetch and performs
 * comparisons. not more to add
 */
void
compute_padding_needed(struct stat sb, UsrOptions *usr_opt)
{
    struct passwd *pwd = getpwuid(sb.st_uid);
    struct group *grp = getgrgid(sb.st_gid);

    int special_file_byte_len = 0;

    int uid_int_len = NbDigitFromInt(sb.st_uid);
    int gid_int_len = NbDigitFromInt(sb.st_gid);

    int nb_byte_len = NbDigitFromInt(sb.st_size);
    int nb_link_len = NbDigitFromInt(sb.st_nlink);


    /* Handle block special or character special file
     * If the file is a block special or character special
     * we need to compute the size of what we will print
     * and save the max value found. which is the major and the minor
     */
    if (S_ISCHR(sb.st_mode) || S_ISBLK(sb.st_mode)) {
        int total_len =
         NbDigitFromInt(major(sb.st_rdev)) + NbDigitFromInt(minor(sb.st_rdev));

        /* +1 to account for the ", " when printing */
        special_file_byte_len = total_len + 2;

        /*
         * Store the stat both in nb_byte_len and special_file_byte_len
         * as it could be used in the human byte field or the classic byte
         * field to print correct padding
         */
        max_check(special_file_byte_len, &(PINFOS->max_special_file_byte_len));
        max_check(special_file_byte_len, &(PINFOS->max_nb_byte_len));
    }


    int nb_block_len = NbDigitFromInt(ComputeBlock(sb.st_blocks));
    int inode_nb_len = NbDigitFromInt(sb.st_ino);

    if (pwd != NULL) {
        max_check(strlen(pwd->pw_name), &(PINFOS->max_uid_str_len));
    }

    if (grp != NULL) {
        max_check(strlen(grp->gr_name), &(PINFOS->max_gid_str_len));
    }

    max_check(uid_int_len, &(PINFOS->max_uid_int_len));

    max_check(gid_int_len, &(PINFOS->max_gid_int_len));

    max_check(nb_byte_len, &(PINFOS->max_nb_byte_len));

    max_check(nb_block_len, &(PINFOS->max_nb_block_len));

    max_check(nb_link_len, &(PINFOS->max_link_nb_len));

    max_check(inode_nb_len, &(PINFOS->max_inode_nb_len));

    if (usr_opt->h) {
        PINFOS->total_bytes += sb.st_size;
    } else {
        PINFOS->total_blocks += sb.st_blocks;
    }
}
