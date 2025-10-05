#ifndef UTILITY_H
#define UTILITY_H

#include "list-handling.h"

char * ConcatTargetToPath(char * path, char * target);
char * RemoveLastFileFromPath(char * path); 
int IsStartingWithDot(char * pathname);
int IsDotDirectory(char * pathname);
int IsHidden(char * pathname);
int CompareMetrics(int metric1, int metric2);
int CompareTimeMetrics(struct timespec t1, struct timespec t2);
char * FullName(char * dirname, char * filename);
void Padding(char * str1, int longest);
int NbDigitFromInt(int val);
int GetBlockSize(void);
int ComputeBlock(int nb_blocks);
long double ComputeBytes(long double nb_bytes);
int CheckPaddingWithStep(FileList * list, int step);

#endif /* !UTILITY_H */
