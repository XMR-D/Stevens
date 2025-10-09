#include "utility.h"
#include "opt_parser.h"

#define KBSIZE 1024

/* defined in ls.c */
extern UsrOptions *USR_OPT;
extern int BLOCKSIZE;

int
CompareMetrics(int metric1, int metric2)
{
    if (metric1 > metric2) {
        return -1;
    }
    if (metric1 < metric2) {
        return 1;
    } else {
        return 0;
    }
}

int
CompareTimeMetrics(struct timespec t1, struct timespec t2)
{
    if (t1.tv_sec != t2.tv_sec) {
        if (t1.tv_sec > t2.tv_sec) {
            return -1;
        } else {
            return 1;
        }
    } else {
        if (t1.tv_nsec > t2.tv_nsec) {
            return -1;
        }
        if (t1.tv_nsec < t2.tv_nsec) {
            return 1;
        } else {
            return 0;
        }
    }
}

int
NbDigitFromInt(int val)
{
    int nb_digit = 0;

    if (val == 0) {
        return 1;
    }

    while (val > 0) {
        nb_digit++;
        val /= 10;
    }

    return nb_digit;
}

/*
 * From a number of given blocks of 512B, Convert it in unit of
 * BLOCKSIZE, if BLOCKSIZE was not defined for any reasons
 * It falls back to 512B blocks unit
 */
int
ComputeBlock(int nb_blocks)
{
    int nb_bytes;

    if (nb_blocks == 0) {
        return 0;
    }

    nb_bytes = nb_blocks * 512;

    if (USR_OPT->k) {
        /* Divide by 1024 to get KB, override any user given block size */
        return (nb_bytes + KBSIZE - 1) / KBSIZE;
    }

    return (nb_bytes + BLOCKSIZE - 1) / BLOCKSIZE;
}
