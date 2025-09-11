#ifndef LISTING_H
#define LISTING_H

#include <sys/stat.h>

#include "targ-parser.h"

#include "list-handling.h"

int ListFile(char * dir, FileList * filelist, FileList * reclist);

int TargetLProcess(TargList * targ_list);


#endif /* !LISTING_H */