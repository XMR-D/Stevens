#ifndef OPT_PARSER_H
#define OPT_PARSER_H

/*
 * Packed structure representing user-passed command line options.
 * Using a packed structure enables efficient bit-level operations.
 * We will be able to convert to/from integer for easy storage and comparison,
 * have fixed-size bit fields that allow clean bitmask operations,
 * and Memory-efficient representation of multiple boolean flags.
 * to me this structure provides a clean, lightweight way to handle options
 */
typedef struct optTokens {

    /* Scope Options */
    char A: 1;  /* Show all entries except '.' and '..' (includes hidden files) */
    char a: 1;  /* Include directory entries whose names begin with a dot ('.') */
    char R: 1;  /* Recursively list subdirectories encountered */
    char d: 1;  /* Directories are listed as plain files (not searched recursively) */
    
    /* Sorting Options */
    char f: 1;  /* Output is not sorted */
    char r: 1;  /* Reverse the order of the sort */
    char t: 1;  /* Sort by time modified (most recent first) */
    char S: 1;  /* Sort by size, largest file first */
    char c: 1;  /* Use time when file status was last changed for sorting/printing */
    char u: 1;  /* Use time of last access for sorting/printing */
    
    /* Format Options */
    char F: 1;  /* Append indicators (/=*|%@) to entries based on file type */
    char l: 1;  /* List in long format with detailed information */
    char n: 1;  /* Long format with numeric owner/group IDs (instead of names) */
    char q: 1;  /* Force printing of non-printable characters as '?' (terminal default) */
    char w: 1;  /* Force raw printing of non-printable characters (non-terminal default) */
    char i: 1;  /* Print file's inode number for each file */

    /* Size format Options*/
    char h: 1;  /* Human readable format (bytes, KB, MB and GB)*/
    char k: 1;  /* Size in kilobytes*/
    char s: 1;  /* Size in 512-byte blocks*/

    /* Special Options*/
    char Help: 1;

} __attribute__((packed)) UsrOptions;


/* 
 * OptSet will check token validity for option candidate
 * Then use bitmask to setup proper options like specified in ls(2) ref page
 * If any error is encountered, it will be immediatly thrown 
 * and all structures will be freed.
 */
int OptSet(char * token);

#endif /* OPT_PARSER_H */