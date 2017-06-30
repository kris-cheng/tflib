#ifndef _TF_FILE_H_
#define _TF_FILE_H_

#define MYPATHCHR       '/'
#define MYPATHSTR       "/"
#define MYEXTCHR        '.'
#define MYEXTSTR        "."
#define MYCDIRSTR       "."
#define MYPDIRSTR       ".."


/* Get the canonicalized absolute path of a file.
   `path' specifies the path of the file.
   The return value is the canonicalized absolute path of a file, or `NULL' if the path is
   invalid.
   Because the region of the return value is allocated with the `malloc' call, it should be
   released with the `free' call when it is no longer in use. */
char *tf_fullpath(const char *path);

/* Get the status information of a file.
   `path' specifies the path of the file.
   `isdirp' specifies the pointer to a variable into which whether the file is a directory is
   assigned.  If it is `NULL', it is ignored.
   `sizep' specifies the pointer to a variable into which the size of the file is assigned.  If
   it is `NULL', it is ignored.
   `ntimep' specifies the pointer to a variable into which the size of the file is assigned.  If
   it is `NULL', it is ignored.
   If successful, the return value is true, else, it is false. */

bool tf_statfile(const char *path, bool *isdirp, int64_t *sizep, int64_t *mtimep);
/* Read whole data of a file.
   `path' specifies the path of the file.  If it is `NULL', the standard input is specified.
   `limit' specifies the limiting size of reading data.  If it is not more than 0, the limitation
   is not specified.
   `sp' specifies the pointer to the variable into which the size of the region of the return
   value is assigned.  If it is `NULL', it is not used.
   The return value is the pointer to the allocated region of the read data, or `NULL' if the
   file could not be opened.
   Because an additional zero code is appended at the end of the region of the return value, the
   return value can be treated as a character string.  Because the region of the return value is
   allocated with the `malloc' call, it should be released with the `free' call when when is no
   longer in use.  */
void *tf_readfile(const char *path, int limit, int *sp);

/* Read every line of a file.
   `path' specifies the path of the file.  If it is `NULL', the standard input is specified.
   The return value is a list object of every lines if successful, else it is `NULL'.
   Line separators are cut out.  Because the object of the return value is created with the
   function `tf_listnew', it should be deleted with the function `tf_listdel' when it is no longer
   in use. */
tf_list_t tf_readlines(const char *path);


/* Write data into a file.
   `path' specifies the path of the file.  If it is `NULL', the standard output is specified.
   `ptr' specifies the pointer to the data region.
   `size' specifies the size of the region.
   If successful, the return value is true, else, it is false. */
bool tf_writefile(const char *path, const void *ptr, int size);


/* Copy a file.
   `src' specifies the path of the source file.
   `dest' specifies the path of the destination file.
   The return value is true if successful, else, it is false.
   If the destination file exists, it is overwritten. */
bool tf_copyfile(const char *src, const char *dest);


/* Read names of files in a directory.
   `path' specifies the path of the directory.
   The return value is a list object of names if successful, else it is `NULL'.
   Links to the directory itself and to the parent directory are ignored.
   Because the object of the return value is created with the function `tclistnew', it should
   be deleted with the function `tclistdel' when it is no longer in use. */
tf_list_t tf_readdir(const char *path);

/* Remove a file or a directory and its sub ones recursively.
   `path' specifies the path of the link.
   If successful, the return value is true, else, it is false.  False is returned when the link
   does not exist or the permission is denied. */
bool tf_removelink(const char *path);

/* Write data into a file.
   `fd' specifies the file descriptor.
   `buf' specifies the buffer to be written.
   `size' specifies the size of the buffer.
   The return value is true if successful, else, it is false. */
bool tf_write(int fd, const void *buf, size_t size);


/* Read data from a file.
   `fd' specifies the file descriptor.
   `buf' specifies the buffer to store into.
   `size' specifies the size of the buffer.
   The return value is true if successful, else, it is false. */
bool tf_read(int fd, void *buf, size_t size);


/* Lock a file.
   `fd' specifies the file descriptor.
   `ex' specifies whether an exclusive lock or a shared lock is performed.
   `nb' specifies whether to request with non-blocking.
   The return value is true if successful, else, it is false. */
bool tf_lock(int fd, bool ex, bool nb);

/* Unlock a file.
   `fd' specifies the file descriptor.
   The return value is true if successful, else, it is false. */
bool tf_unlock(int fd);

#endif
