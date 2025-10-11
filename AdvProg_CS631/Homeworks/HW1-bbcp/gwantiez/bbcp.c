/*
 * Copyright (c) 2025 Guillaume Wantiez (alias XMR)
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 
 * Define a fixed size to create later a reading buffer 
 * 4096 because it's the default st_blocksize for many filesystem
 */
#define READ_BUF_SIZE 4096

/*
 * Routine that take an input string and checks
 * whenever this string represent a valid path for copy purposes
 * or not.
 *
 * that means a valid string represent :
 * 	- String that represent a non exisiting file
 * 	- String that represent an existing file
 * 	- String that represent an existing directory 
 * 	- Path leading through existing directories to a non existing file
 * 	- Path leading through existing directories to an existing file
 * 
 * every other strings are considered invalid
 *
 * Note: This function is always called on destination paths,
 * as source paths checks are performed before any call to this routine.
 */
static int 
CheckIfPathValid(char* str) 
{
    char* first_slash = strchr(str, '/');
    char* last_chunk;
    char* saved;

    /* Path is valid since it's a file*/
    if (first_slash == NULL) {
	    return 0;
    }

    last_chunk = strrchr(str, '/');
    saved = last_chunk;

    last_chunk++;

    /* Path is representing a dir */
    if (*last_chunk == '\0') {
        struct stat sb;

        if (stat(str, &sb)) {
            fprintf(stderr, "error: bbcp: %s.\n", strerror(errno));
            return errno;
        }
        return 0;
    }
    /* Path is representing a multidir path to a file */
    else {
        /* Truncate the string before the filename*/
        char c_save = *saved;
        struct stat sb;
        
	*saved = '\0';

        if (stat(str, &sb)) {
            fprintf(stderr, "error: bbcp: %s.\n", strerror(errno));
            return errno;
        }

        *saved = c_save;
        return 0;
    }
}


static char *
CraftFullDestPath(char * src_file, char * dest_dir)
{
    int destlen, srclen, fullpath_len;
    char * fullpath;

    destlen = strlen(dest_dir);
    srclen = strlen(src_file);
    
    if (dest_dir[destlen-1] == '/') {
	dest_dir[destlen-1] = '\0';
    }
    
    fullpath_len = destlen + srclen + 2;
    fullpath = calloc(sizeof(char), fullpath_len);

    if (fullpath == NULL) {
        fprintf(stderr, "error: bbcp: %s\n", strerror(errno));
        return NULL;
    }

    if (snprintf(fullpath, fullpath_len, "%s/%s", dest_dir, src_file) < 0) {
        fprintf(stderr, "error: bbcp: critical error in snprintf\n");
        free(fullpath);
        return NULL;
    }

    return fullpath;
}

/*
 * Routine that check inode and device ID 
 * if both dest and source are the same,
 * throw an error files are identicals
 *
 * Note: None
 */
static int 
CheckIdentical(char * srcpath, char * destpath)
{
    struct stat sb1, sb2;
    char * fullpath;
    char * tmp;
    int fullpath_free = 0;
  
    if (srcpath == NULL || destpath == NULL) {
        return EXIT_FAILURE;
    }

    if (stat(srcpath, &sb1) == -1) {
	fprintf(stderr, "error: bbcp: %s\n", strerror(errno));
	return errno;
    }

    /* 
     * Check destpath before any modification to check if it's a directory
     * or not, if it's a directory craft a new path, if not just check if 
     * files are the same
     */
    if (stat(destpath, &sb2) == -1) {
	
	if (errno != ENOENT) {
	    /* Silence the error in case the file does not exist, since we will create it */
 	    fprintf(stderr, "error: bbcp: %s\n", strerror(errno));
	    return errno;
	}
    }

    tmp = strrchr(srcpath, '/');

    if (tmp != NULL) {
        tmp++;
	srcpath = tmp;
    }


    if (S_ISDIR(sb2.st_mode)) {

        fullpath = CraftFullDestPath(srcpath, destpath);
	if (fullpath == NULL) {
	    return errno;
	}
	fullpath_free = 1;

    	if (stat(fullpath, &sb2) == -1) {
	    /* Silence the error in case the file does not exist, since we will create it */
            if (errno != ENOENT) {
	        fprintf(stderr, "error: bbcp: %s\n", strerror(errno));
	        return errno;
	    }
        }
    } else {
        fullpath = destpath;
    } 

    if ((sb1.st_ino == sb2.st_ino) && 
		    (sb1.st_dev == sb2.st_dev)) {

        fprintf(stderr, "error: bbcp: %s and %s are identical. (not copied).\n", srcpath, fullpath);
	
	if (fullpath_free) {
	   free(fullpath);
	}
        return EXIT_SUCCESS;
    }
    
    if (fullpath_free) {
        free(fullpath);
    }
    return EXIT_FAILURE; 
}

/*
 * Routine that copy a file named src_file overwriting or creating the file
 * named dest_file.
 *
 * Note: None
 */
static int 
CopyFile(char* src_file, char* dest_file) {
    /*
     * Open both source and fullpath of destination
     * source on read only
     * dest on write only + create the file if non existant
     */
    int src_fd, dest_fd;
    char read_buf[READ_BUF_SIZE] = { 0 };
    size_t read_size = 0;
    struct stat sb;
    
    /* Open file as byte mode read only */
    src_fd = open(src_file, O_RDONLY | O_REGULAR);
    if (src_fd == -1) {
        fprintf(stderr, "error: bbcp: %s.\n", strerror(errno));
        return errno;
    } 

    /* Fetch the permissions */
    if (stat(src_file, &sb)) {
        fprintf(stderr, "error: bbcp: Failed to fetch permissions.\n");
	return errno;
    }

    /* Open file as byte mode write only and create if non existent */
    dest_fd = open(dest_file, O_WRONLY | O_CREAT | O_TRUNC, sb.st_mode);
    if (dest_fd == -1) {
        fprintf(stderr, "error: bbcp: %s.\n", strerror(errno));
        return errno;
    }
    
    /*
     * While lines can be read, read them from src file descriptor
     * and write them to the dest file descriptor
     */
    while ((read_size = read(src_fd, read_buf, READ_BUF_SIZE)) > 0) {
        if (write(dest_fd, read_buf, read_size) == -1) {
	    fprintf(stderr, "error: bbcp: %s.\n", strerror(errno));
	    return errno;
	}
    }
    close(src_fd);
    close(dest_fd);

    return EXIT_SUCCESS;
}

/*
 * Routine that copy a file named src_file at path src_path
 * into the directory at path dest_path.
 *
 * Note: the routine concatenate the source path to the destination path
 * before performing the copy. src_file is inteded to be stripped of any
 * redirections (../../dir/file ==> file).
 */
static int 
CopyToADir(char* src_file, char * src_path, char* dest_dir) {
    /*
     * Craft the fullpath to the directory
     * 'src = toto | dir = path/to/dir => 'fullpath = path/to/dir/toto'
     * +2 because of the '/' and the '\0' we add.
     */
    char * fullpath = CraftFullDestPath(src_file, dest_dir);
    if (fullpath == NULL) {
        return errno;
    }
 
    if (CopyFile(src_path, fullpath)) {
        free(fullpath);
        return errno;
    }

    free(fullpath);
    return EXIT_SUCCESS;
}

/* bbcp main routine
 *
 * Copy file named src_file at path src_path to path dest_path
 *
 * return an int, 0 on success, >0 on failure
 *
 * Note: main() already check for the the number of arguments 
 * and the validity of the source
 */
static int 
bbcp(char * src_file, char* src_path, char* dest_path) 
{
    struct stat sb;
    int dest_stat_fail;

    if (src_path == NULL || dest_path == NULL) {
        fprintf(stderr, "error: bbcp: argument format error.\n");
        return EINVAL;
    }

    if (CheckIfPathValid(dest_path)) {
        return errno;
    }

    if (CheckIdentical(src_path, dest_path) == 0) {
	
	/*
	 * If errno has not been set return an invalid argument error code
	 * else errno has been properly set by another function so return
	 * this error code instead.
	 */
	if (errno == 0) {
	    return EINVAL;
	}
	return errno;
    }


    dest_stat_fail = stat(dest_path, &sb);

    /* Dest_path is a file or does not exist */
    if (dest_stat_fail || S_ISREG(sb.st_mode)) { 
	    return CopyFile(src_path, dest_path);
    }

    /* Dest_path is a directory */
    else if (S_ISDIR(sb.st_mode)) {
        return CopyToADir(src_file, src_path, dest_path);
    }

    /* else Dest_path exist but it's not a directory nor a regular file */
    fprintf(stderr,
        "error: bbcp: source file is not a regular filen nor a directory.\n");
    return EINVAL;
}

/*
 * Entry point of the executable 
 * Taking argc and argv from stack or registers depending
 * on the system ABI
 *
 * Note: If you want to export bbcp to a shared library,
 * - Rename this routine for bbcp
 * - Add the  __attribute__((visibility("default"))) to mark the
 *   function accessible outside the shared library.
 * - Remove the setprogname(argv[0]) line
 * - Create an header files to export this routine only
 * - Compile as a shared library
 */
int main(int argc, char** argv) 
{
    setprogname(argv[0]);
    
    if (argc == 3) {

        struct stat sb;
	char * filename;

        if (stat(argv[1], &sb)) {
            fprintf(stderr, "error: bbcp: %s\n", strerror(errno));
            return errno;
        }

        if (S_ISDIR(sb.st_mode)) {
            fprintf(stderr, "error: bbcp: source file %s\n", strerror(EISDIR));
            return EISDIR;
        }

	filename = strrchr(argv[1], '/');

	if (filename == NULL)
		return bbcp(argv[1], argv[1], argv[2]);
	else {
		filename++;
       		return bbcp(filename, argv[1], argv[2]);
	}
    }
 
    fprintf(stderr, "error: bbcp: argument format error\n");
    return EINVAL;
}
