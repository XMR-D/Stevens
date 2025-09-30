#ifndef LISTING_H
#define LISTING_H

#include <sys/stat.h>

#include "targ_parser.h"

#include "list-handling.h"

int ListFile(char * dir, FileList * filelist, FileList * reclist);
void 
ResetPinfos(PrintInfos * infos);

void
ComputePaddingNeeded(FileList * elm);

int 
TargetLProcess(TargList * targets);


#endif /* !LISTING_H */
