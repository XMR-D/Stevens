#include "padding-handling.h"

#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"
#include "opt_parser.h"
#include "utility.h"

/* PrintInfos structure that will be used later in printing.c */
extern PaddingInfos *PINFOS;

void
Padding(char *str1, int longest)
{

    int spaces = longest - strlen(str1);

    while (spaces > 0) {
        printf(" ");
        spaces--;
    }
}

void
ResetPrintInfos(PaddingInfos *infos)
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
MaxCheck(int val, int *max)
{
    if (val > *max) {
        *max = val;
    }
}

/*
 * Compute the length of all the possible numerical fields
 * of the long format printer for all elments.
 *
 * Compare it to the pinfos actual structure that needs to contain
 * all the max lengths to get a proper output
 */
void
ComputePaddingNeeded(struct stat sb, UsrOptions *usr_opt)
{
    struct passwd *pwd = getpwuid(sb.st_uid);
    struct group *grp = getgrgid(sb.st_gid);

    int uid_int_len = NbDigitFromInt(sb.st_uid);
    int gid_int_len = NbDigitFromInt(sb.st_gid);

    int nb_byte_len = NbDigitFromInt(sb.st_size);
    int nb_link_len = NbDigitFromInt(sb.st_nlink);
    int nb_block_len = NbDigitFromInt(ComputeBlock(sb.st_blocks));
    int inode_nb_len = NbDigitFromInt(sb.st_ino);

    if (pwd != NULL) {
        MaxCheck(strlen(pwd->pw_name), &(PINFOS->max_uid_str_len));
    }

    if (grp != NULL) {
        MaxCheck(strlen(grp->gr_name), &(PINFOS->max_gid_str_len));
    }

    MaxCheck(uid_int_len, &(PINFOS->max_uid_int_len));

    MaxCheck(gid_int_len, &(PINFOS->max_gid_int_len));

    MaxCheck(nb_byte_len, &(PINFOS->max_nb_byte_len));

    MaxCheck(nb_link_len, &(PINFOS->max_link_nb_len));

    MaxCheck(nb_block_len, &(PINFOS->max_nb_block_len));

    MaxCheck(inode_nb_len, &(PINFOS->max_inode_nb_len));

    if (usr_opt->h) {
        PINFOS->total_bytes += sb.st_size;
    } else {
        PINFOS->total_blocks += sb.st_blocks;
    }
}
