#include "utility.h"

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "opt_parser.h"

#ifndef SIZES
#define BSIZE 1
#define KBSIZE 1024
#define MBSIZE 1048576
#define GBSIZE 1073741824
#define TBSIZE 1099511627776
#define SIZES

#endif /* ! SIZES */
#ifndef DEFAULT_BLK_SIZE
#define DEFAULT_BLK_SIZE 512
#endif /* !DEFAULT_BLK_SIZE */

#define MIN_BLK_SIZE DEFAULT_BLK_SIZE
#define MAX_BLK_SIZE GBSIZE


extern int BLOCKSIZE;
extern UsrOptions *USR_OPT;

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

/*
 * From nb_bytes bytes, print and return the number of bytes
 * in the most apropriate unit.
 */
long double
ComputeBytes(long double nb_bytes)
{
    long int unit = 0;

    if (nb_bytes == 0) {
        return 0;
    }

    if (nb_bytes != 0) {
        if (nb_bytes < KBSIZE) {
            unit = BSIZE;
        } else if (nb_bytes >= KBSIZE && nb_bytes < MBSIZE) {
            unit = KBSIZE;
        } else if (nb_bytes >= MBSIZE && nb_bytes < GBSIZE) {
            unit = MBSIZE;
        } else if (nb_bytes >= GBSIZE && nb_bytes < TBSIZE) {
            unit = GBSIZE;
        } else if (nb_bytes >= TBSIZE) {
            unit = TBSIZE;
        }
    }

    return nb_bytes / unit;
}
