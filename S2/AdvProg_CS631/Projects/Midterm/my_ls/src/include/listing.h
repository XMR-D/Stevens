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
DirectoriesProcess(TargList * directories_list);

int 
FilesProcess(TargList * file_list);

#endif /* !LISTING_H */
