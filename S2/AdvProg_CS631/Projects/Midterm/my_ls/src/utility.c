#include "utility.h"

#include "opt_parser.h"

#define KBSIZE 1024

/* defined in ls.c */
extern UsrOptions *USR_OPT;
extern int BLOCKSIZE;

/* CompareMetrics : Utility routine that compare two integer value 
 * representing two metrics 
 *
 * return -1 if the first one is bigger than the second one
 * return 1 if the first one is smaller than the second one
 * return 0 if both are equal
 *
 * Note : (None)
 */
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

/* CompareTimeMetrics : Utility routine that compare two integer value 
 * representing two time metrics
 *
 * return -1 if the first one is bigger than the second one
 * return 1 if the first one is smaller than the second one
 * return 0 if both are equal
 *
 * Note : (None)
 */
int
CompareTimeMetrics(struct timespec t1, struct timespec t2)
{
    /*
     * First compare seconds, if it's the same try to
     * discriminate using nanoseconds for higher precision.
     *
     * If a time metric for sorting is specificaly
     * passed it means that this check is important
     * to the user so we might as well done it precisely.
     */
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

/* NbDigitFromInt : Utility routine that compute the number
 * of digit in val.
 *
 * return the number of digit found.
 *
 * Note : (None)
 */
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
 * ComputeBlock : Utility routine that convert a number of blocks of 512B
 * into blocks of BLOCKSIZE  or in blocks of KBSIZE if k is specified. 
 *
 * Note : (None)
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
