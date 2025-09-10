#ifndef UTILITY_H
#define UTILITY_H

int IsHidden(char * pathname);
int CompareMetrics(int metric1, int metric2);
int CompareTimeMetrics(struct timespec t1, struct timespec t2);
char * FullName(char * dirname, char * filename);
void Padding(char * str1, int longest);
int NbDigit(int val);

#endif /* !UTILITY_H */
