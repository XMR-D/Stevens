#ifndef PRINTING_H
#define PRINTING_H

#include <sys/stat.h>

#include <fts.h>

void PrintTotalBytes(void);
int LongFormatPrinter(FTSENT *parentdir, FTSENT *list);

#endif /* !PRINTING_H */
