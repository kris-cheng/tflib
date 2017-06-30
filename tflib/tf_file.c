#include "tflib.h"
#include "tf_file.h"
#include <sys/stat.h>
#include <fcntl.h> /* open O_RDWR */

#define TFFILEMODE     00644             // permission of a creating file
#define TFIOBUFSIZ     16384             // size of an I/O buffer

/* Get the canonicalized absolute path of a file. */
char *tf_fullpath(const char *path){
    assert(path);
    char buf[PATH_MAX+1];
    if(realpath(path, buf)) return strdup(buf);
    if(errno == ENOENT){
        const char *pv = strrchr(path, MYPATHCHR);
        if(pv){
            if(pv == path) return strdup(path);
            char *prefix = tf_memdup(path, pv - path);
            if(!realpath(prefix, buf)){
                tf_free(prefix);
                return NULL;
            }
            tf_free(prefix);
            pv++;
        } else {
            if(!realpath(MYCDIRSTR, buf)) return NULL;
            pv = path;
        }
        if(buf[0] == MYPATHCHR && buf[1] == '\0') buf[0] = '\0';
        char *str = tf_alloc(strlen(buf) + strlen(pv) + 2);
        sprintf(str, "%s%c%s", buf, MYPATHCHR, pv);
        return str;
    }
    return NULL;
}

/* Get the status information of a file. */
bool tf_statfile(const char *path, bool *isdirp, int64_t *sizep, int64_t *mtimep){
    assert(path);
    struct stat sbuf;
    if(stat(path, &sbuf) != 0) return false;
    if(isdirp) *isdirp = S_ISDIR(sbuf.st_mode);
    if(sizep) *sizep = sbuf.st_size;
    if(mtimep) *mtimep = sbuf.st_mtime;
    return true;
}

/* Read whole data of a file. */
void *tf_readfile(const char *path, int limit, int *sp){
    int fd = path ? open(path, O_RDONLY, TFFILEMODE) : 0;
    if(fd == -1) return NULL;
    if(fd == 0){
        tf_str_t xstr = tf_strnew();
        char buf[TFIOBUFSIZ];
        limit = limit > 0 ? limit : INT_MAX;
        int rsiz;
        while((rsiz = read(fd, buf, min(TFIOBUFSIZ, limit))) > 0){
            tf_strcat(xstr, buf, rsiz);
            limit -= rsiz;
        }
        if(sp) *sp = tf_strsize(xstr);
        return tf_strtomalloc(xstr);
    }

    struct stat sbuf;
    if(fstat(fd, &sbuf) == -1 || !S_ISREG(sbuf.st_mode)){
        close(fd);
        return NULL;
    }
    limit = limit > 0 ? min((int)sbuf.st_size, limit) : sbuf.st_size;
    char *buf = tf_alloc( sbuf.st_size + 1);
    char *wp = buf;
    int rsiz;
    while((rsiz = read(fd, wp, limit - (wp - buf))) > 0){
        wp += rsiz;
    }

    *wp = '\0';
    close(fd);
    if(sp) *sp = wp - buf;
    return buf;
}

/* Read every line of a file. */
tf_list_t tf_readlines(const char *path){
    int fd = path ? open(path, O_RDONLY, TFFILEMODE) : 0;
    if(fd == -1) return NULL;
    tf_list_t list = tf_list_new();
    tf_str_t  xstr = tf_strnew();
    char buf[TFIOBUFSIZ];
    int rsiz;
    while((rsiz = read(fd, buf, TFIOBUFSIZ)) > 0){
        for(int i = 0; i < rsiz; i++){
            switch(buf[i]){
            case '\r':
                break;
            case '\n':
                tf_list_push(list, tf_strptr(xstr), tf_strsize(xstr));
                tf_strclear(xstr);
                break;
            default:
                tf_strcat(xstr, buf + i, 1);
                break;
            }
        }
    }
    tf_list_push(list, tf_strptr(xstr), tf_strsize(xstr));
    tf_strdel(xstr);
    if(path) close(fd);
    return list;
}

/* Write data into a file. */
bool tf_writefile(const char *path, const void *ptr, int size){
    assert(ptr && size >= 0);
    int fd = 1;
    if(path && (fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, TFFILEMODE)) == -1) return false;
    bool err = false;
    if(!tf_write(fd, ptr, size)) err = true;
    if(close(fd) == -1) err = true;
    return !err;
}

/* Copy a file. */
bool tf_copyfile(const char *src, const char *dest){
    int ifd = open(src, O_RDONLY, TFFILEMODE);
    if(ifd == -1) return false;
    int ofd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, TFFILEMODE);
    if(ofd == -1){
        close(ifd);
        return false;
    }
    bool err = false;
    while(true){
        char buf[TFIOBUFSIZ];
        int size = read(ifd, buf, TFIOBUFSIZ);
        if(size > 0){
            if(!tf_write(ofd, buf, size)){
                err = true;
                break;
            }
        } else if(size == -1){
            if(errno != EINTR){
                err = true;
                break;
            }
        } else {
            break;
        }
    }
    if(close(ofd) == -1) err = true;
    if(close(ifd) == -1) err = true;
    return !err;
}

/* Read names of files in a directory. */
tf_list_t tf_readdir(const char *path){
    assert(path);
    DIR *DD;
    struct dirent *dp;
    if(!(DD = opendir(path))) return NULL;

    tf_list_t list = tf_list_new();
    while((dp = readdir(DD)) != NULL){
        if(!strcmp(dp->d_name, MYCDIRSTR) || !strcmp(dp->d_name, MYPDIRSTR)) continue;
        tf_list_push(list, dp->d_name, strlen(dp->d_name));
    }
    closedir(DD);
    return list;
}

/* Remove a file or a directory and its sub ones recursively. */
bool tf_removelink(const char *path){
    assert(path);
    struct stat sbuf;
    if(lstat(path, &sbuf) == -1) return false;
    if(unlink(path) == 0) return true;
    tf_list_t list;
    if(!S_ISDIR(sbuf.st_mode) || !(list = tf_readdir(path))) return false;
    bool tail = path[0] != '\0' && path[strlen(path)-1] == MYPATHCHR;
    for(int i = 0; i < tf_list_num(list); i++){
        const char *elem = tf_list_val2(list, i);
        if(!strcmp(MYCDIRSTR, elem) || !strcmp(MYPDIRSTR, elem)) continue;

        char *cpath;
        if(tail){
            cpath = tf_sprintf("%s%s", path, elem);
        } else {
            cpath = tf_sprintf("%s%c%s", path, MYPATHCHR, elem);
        }
        tf_removelink(cpath);
        tf_free(cpath);
    }
    tf_list_del(list);
    return rmdir(path) == 0 ? true : false;
}

/* Write data into a file. */
bool tf_write(int fd, const void *buf, size_t size){
    assert(fd >= 0 && buf && size >= 0);
    const char *rp = buf;
    do {
        int wb = write(fd, rp, size);
        switch(wb){
            case -1: if(errno != EINTR) return false;
            case 0: break;
            default:
                rp += wb;
                size -= wb;
                break;
        }
    } while(size > 0);
    return true;
}

/* Read data from a file. */
bool tf_read(int fd, void *buf, size_t size){
    assert(fd >= 0 && buf && size >= 0);
    char *wp = buf;
    do {
        int rb = read(fd, wp, size);
        switch(rb){
            case -1: if(errno != EINTR) return false;
            case 0: return size < 1;
            default:
                wp += rb;
                size -= rb;
        }
    } while(size > 0);
    return true;
}

/* Lock a file. */
bool tf_lock(int fd, bool ex, bool nb){
    assert(fd >= 0);
    struct flock lock;
    memset(&lock, 0, sizeof(struct flock));
    lock.l_type = ex ? F_WRLCK : F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = 0;
    while(fcntl(fd, nb ? F_SETLK : F_SETLKW, &lock) == -1){
        if(errno != EINTR) return false;
    }
    return true;
}

/* Unlock a file. */
bool tf_unlock(int fd){
    assert(fd >= 0);
    struct flock lock;
    memset(&lock, 0, sizeof(struct flock));
    lock.l_type = F_UNLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_pid = 0;
    while(fcntl(fd, F_SETLKW, &lock) == -1){
        if(errno != EINTR) return false;
    }
    return true;
}
