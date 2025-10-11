## Ls(1) - List Directory Contents

### SYNOPSIS
**ls** [−AacdFfhiklnqRrSstuw] [file...]

### DESCRIPTION
For each operand that names a file of a type other than directory, **ls** displays its name as well as any requested, associated information. For each operand that names a file of type directory, **ls** displays the names of files contained within that directory, as well as any requested, associated information.

If no operands are given, the contents of the current directory are displayed. If more than one operand is given, non-directory operands are displayed first; directory and non-directory operands are sorted separately and in lexicographical order.

### OPTIONS

**-A**  
  List all entries except for '.' and '..'. Always set for the super-user.

**-a**  
  Include directory entries whose names begin with a dot (.).

**-c**  
  Use time when file status was last changed, instead of time of last modification of the file for sorting (-t) or printing (-l).

**-d**  
  Directories are listed as plain files (not searched recursively) and symbolic links in the argument list are not indirected through.

**-F**  
  Display a slash (/) immediately after each pathname that is a directory, an asterisk (*) after each that is executable, an at sign (@) after each symbolic link, a percent sign (%) after each whiteout, an equal sign (=) after each socket, and a vertical bar (|) after each that is a FIFO.

**-f**  
  Output is not sorted.

**-h**  
  Modifies the -s and -l options, causing the sizes to be reported in bytes displayed in a human readable format. Overrides -k.

**-i**  
  For each file, print the file's file serial number (inode number).

**-k**  
  Modifies the -s option, causing the sizes to be reported in kilobytes. The rightmost of the -k and -h flags overrides the previous flag. See also -h.

**-l**  
  (The lowercase letter "ell"). List in long format. (See below.)

**-n**  
  The same as -l, except that the owner and group IDs are displayed numerically rather than converting to a owner or group name.

**-q**  
  Force printing of non-printable characters in file names as the character '?'; this is the default when output is to a terminal.

**-R**  
  Recursively list subdirectories encountered.

**-r**  
  Reverse the order of the sort to get reverse lexicographical order or the smallest or oldest entries first.

**-S**  
  Sort by size, largest file first.

**-s**  
  Display the number of file system blocks actually used by each file, in units of 512 bytes or BLOCKSIZE (see ENVIRONMENT) where partial units are rounded up to the next integer value. If the output is to a terminal, a total sum for all the file sizes is output on a line before the listing.

**-t**  
  Sort by time modified (most recently modified first) before sorting the operands by lexicographical order.

**-u**  
  Use time of last access, instead of last modification of the file for sorting (-t) or printing (-l).

**-w**  
  Force raw printing of non-printable characters. This is the default when output is not to a terminal.

### OPTION INTERACTIONS

- The **-w** and **-q** options override each other; the last one specified determines the format used for non-printable characters.
- The **-l** and **-n** options override each other; the last one specified determines the format used.
- The **-c** and **-u** options override each other; the last one specified determines the file time used.

By default, **ls** lists one entry per line to standard output. File information is displayed with one or more blanks separating the information associated with the -i, -s, and -l options.

### THE LONG FORMAT

If the **-l** option is given, the following information is displayed for each file:

- File mode
- Number of links
- Owner name
- Group name
- Number of bytes in the file
- Abbreviated month file was last modified
- Day-of-month file was last modified
- Hour and minute file was last modified
- Pathname

In addition, for each directory whose contents are displayed, the total number of blocks in units of 512 bytes or BLOCKSIZE (see ENVIRONMENT) used by the files in the directory is displayed on a line by itself immediately before the information for the files in the directory.

If the owner or group names are not a known owner or group name, or the **-n** option is given, the numeric IDs are displayed.

If the file is a character special or block special file, the major and minor device numbers for the file are displayed in the size field. If the file is a symbolic link the pathname of the linked-to file is preceded by "->".

### FILE MODE FORMAT

The file mode printed under the **-l** option consists of the entry type, owner permissions, group permissions, and other permissions. The entry type character describes the type of file:

- **-** - Regular file
- **d** - Directory
- **l** - Symbolic link
- **b** - Block special file
- **c** - Character special file
- **s** - Socket link
- **p** - FIFO
- **w** - Whiteout
- **a** - Archive state 1
- **A** - Archive state 2

The next three fields are three characters each: owner permissions, group permissions, and other permissions. Each field has three character positions:

1. If **r**, the file is readable; if **-**, it is not readable.
2. If **w**, the file is writable; if **-**, it is not writable.
3. The first of the following that applies:
   - **S** - If in owner permissions: file not executable + set-user-ID mode set. If in group permissions: file not executable + set-group-ID mode set.
   - **s** - If in owner permissions: file executable + set-user-ID mode set. If in group permissions: file executable + set-group-ID mode set.
   - **x** - File is executable or directory is searchable.
   - **-** - File is neither readable, writable, executable, nor set-user-ID nor set-group-ID mode, nor sticky.

These next two apply only to the third character in the last group (other permissions):

- **T** - Sticky bit set but not execute or search permission
- **t** - Sticky bit set and is searchable or executable

### ENVIRONMENT

The following environment variables affect the execution of **ls**:

- **BLOCKSIZE** - If set, and the -h and -k options are not specified, the block counts (see -s) will be displayed in units of that size block.
- **TZ** - The timezone to use when displaying dates.

### EXIT STATUS

The **ls** utility exits 0 on success, and >0 if an error occurs.

### COMPATIBILITY

The group field is now automatically included in the long listing for files in order to be compatible with the IEEE Std 1003.2 ("POSIX.2") specification.

### SEE ALSO

chmod(1), stat(2), strmode(3), getbsize(3), dir(5), sticky(7), symlink(7)

### STANDARDS

This **ls** utility is a superset of the IEEE Std 1003.2 ("POSIX.2") specification.

### HISTORY

An **ls** utility appeared in Version 5 AT&T UNIX.

BSD September 12, 2019
