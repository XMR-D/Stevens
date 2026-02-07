/* vi: set sw=4 ts=4: */
/*
 * mode_string implementation for busybox
 *
 * Copyright (C) 2003  Manuel Novoa III  <mjn3@codepoet.org>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 */
#include <assert.h>
#include "libbb.h"

#if ( S_ISUID != 04000 ) || ( S_ISGID != 02000 ) || ( S_ISVTX != 01000 ) \
 || ( S_IRUSR != 00400 ) || ( S_IWUSR != 00200 ) || ( S_IXUSR != 00100 ) \
 || ( S_IRGRP != 00040 ) || ( S_IWGRP != 00020 ) || ( S_IXGRP != 00010 ) \
 || ( S_IROTH != 00004 ) || ( S_IWOTH != 00002 ) || ( S_IXOTH != 00001 )
#error permission bitflag value assumption(s) violated!
#endif

/* Generate ls-style "mode string" like "-rwsr-xr-x" or "drwxrwxrwt" */

#if ( S_IFSOCK!= 0140000 ) || ( S_IFLNK != 0120000 ) \
 || ( S_IFREG != 0100000 ) || ( S_IFBLK != 0060000 ) \
 || ( S_IFDIR != 0040000 ) || ( S_IFCHR != 0020000 ) \
 || ( S_IFIFO != 0010000 )
# warning mode type bitflag value assumption(s) violated! falling back to larger version

# if (S_IRWXU | S_IRWXG | S_IRWXO | S_ISUID | S_ISGID 