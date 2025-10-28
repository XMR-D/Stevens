*Copyright (c) 2025 Guillaume Wantiez (alias XMR)*

*Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software.*

*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*


# Building

To build the program run the following command :

    $> make 

To clean the repository run the following command :

    $> make clean

# Description

#### NAME
stat --- display file status

#### SYNOPSIS
    $> ./stat [file ...]

#### DESCRIPTION
The stat utility displays information about each file given by file.
Read, write, or execute permissions for the named file are not required,
but all directories listed in the pathname leading to the file must be
searchable.

If no file argument is given, stat displays information about the file
descriptor for standard input.

#### EXAMPLES
The following examples show common usage:
       
    $> ./stat file1
    $> ./stat *
    $> ./stat   

#### EXIT STATUS
stat exits 0 on success, and >0 if an error occurred.

#### NOTES
The code for the stat utility is, well-formatted according to the guidelines found 
at https://stevens.netmeister.org/631/style. The program can be compiled using the 
-Wall and -Werror flags without any errors or warnings on a NetBSD 10.x system.


#### IA USAGE

Model Used : Deepseek-V3

Initial prompt :

    *Using the AI engine of your choice, write a simple version of the stat(1) command that 
     behaves as if the -F flag had been given. (Your version will not support any other flags.)

    Your program will:
    follow the general homework guidelines
    apply the coding style and best practices we discussed
    be accompanied by a Makefile, README, and checklist

    here is the stat man page of netbsd:

    NAME

        stat -- display file status


    SYNOPSIS

        stat [-FLnq] [-f format | -l | -r | -s | -x] [-t timefmt] [file ...]


    DESCRIPTION

        The stat utility displays information about each file given by file.
        Read, write, or execute permissions for the named file are not required,
        but all directories listed in the pathname leading to the file must be
        searchable.
        If no file argument is given, stat displays information about the file
        descriptor for standard input.  In this case the -L option is ignored,
        and stat uses fstat(2) rather than lstat(2) or stat(2) to obtain informa-
        tion.  The `file name' (and also the `path name') in this case is
        `(stdin)'.  The file number (`%@') will be zero.

        Otherwise the information displayed is obtained by calling lstat(2) (or
        stat(2) with -L) with each given argument in turn and evaluating the
        returned structure.

        The default format displays the st_dev, st_ino, st_mode, st_nlink,
        st_uid, st_gid, st_rdev, st_size, st_atime, st_mtime, st_ctime,
        st_birthtime, st_blksize, st_blocks, and st_flags fields, in that order.

        The options are as follows:

        -F          As in ls(1), display a slash (`/') immediately after each
                    pathname that is a directory, an asterisk (`*') after each
                    that is executable, an at sign (`@') after each symbolic
                    link, a percent sign (`%') after each whiteout, an equal sign
                    (`=') after each socket, and a vertical bar (`|') after each
                    that is a FIFO.  The use of -F implies -l.*








Method : 

Once a first skeletton generated, I arranged the code to fit the coding style and
to fit more the program description, then run comparative tests

Then for each modification in the logic I asked the model to regenerate me parts of the
code that was uterly wrong.

Finaly when the code was finally working the way it was inteded I had to refactor parts
of it to harden it a little, reduce the sizes of buffer, privillege a non dynamic aproach
to limit memory corruption or leaks and add some error checking, change C standard functions to some 
limited one strcpy = strncpy for example and finally change some AI generated implementation
of things already existing in the C standard.


CONCLUSION : 
AI is a big no no to make computers bip bip boup bap I hate doing work with AI
