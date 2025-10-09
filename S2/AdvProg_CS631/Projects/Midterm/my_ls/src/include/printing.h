#ifndef PRINTING_H
#define PRINTING_H

#include <sys/stat.h>

#include <fts.h>

int long_format_printer(FTSENT *parentdir, FTSENT *list);

#endif /* !PRINTING_H */
