#ifndef _TF_CONFIG_LINUX_H_
#define _TF_CONFIG_LINUX_H_

#ifndef _GNU_SOURCE
#define _GNU_SOURCE  1        /* pread(), pwrite(), gethostname() */
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>             /* offsetof() */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <glob.h>
#include <sys/vfs.h>            /* statfs() */

#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sched.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>        /* TCP_NODELAY, TCP_CORK */
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>

#include <time.h>               /* tzset() */
#include <malloc.h>             /* memalign() */
#include <limits.h>             /* IOV_MAX */
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/syscall.h>  
#include <crypt.h>
#include <sys/utsname.h>        /* uname() */


#if (TF_HAVE_POLL || TF_HAVE_RTSIG)
#include <poll.h>
#endif

#if (TF_HAVE_EPOLL)
#include <sys/epoll.h>
#endif

inline pid_t gettid(void){   return syscall(SYS_gettid);  } 

#define TF_LISTEN_BACKLOG        511

#if defined TCP_DEFER_ACCEPT && !defined TF_HAVE_DEFERRED_ACCEPT
#define TF_HAVE_DEFERRED_ACCEPT  1
#endif

#define _XOPEN_SOURCE 700

#endif /* _TF_CONFIG_LINUX_H_ */
