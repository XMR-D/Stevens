#ifndef UTILITY_H
#define UTILITY_H

#include <time.h>

int CompareMetrics(int metric1, int metric2);
int CompareTimeMetrics(struct timespec t1, struct timespec t2);
int NbDigitFromInt(int val);
int ComputeBlock(int nb_blocks);
long double ComputeBytes(long double nb_bytes);

#endif /* !UTILITY_H */
