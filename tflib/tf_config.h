#ifndef _TF_CONFIG_H_
#define _TF_CONFIG_H_

#if defined(__cplusplus)
#define __TF_CLINKAGEBEGIN extern "C" {
#define __TF_CLINKAGEEND }
#else
#define __TF_CLINKAGEBEGIN
#define __TF_CLINKAGEEND
#endif

__TF_CLINKAGEBEGIN

#if ! defined(__cplusplus)
#include <stdbool.h>
#endif

#ifndef bool
typedef unsigned char bool;
#define true      1
#define false     0
#define YES       true
#define NO        false
#define TRUE      YES
#define FALSE     NO
#endif

#ifndef __u8
typedef unsigned char   __u8;
typedef unsigned short  __u16;
typedef unsigned int    __u32;
#endif

#define  TF_OK          0
#define  TF_ERROR      -1
#define  TF_AGAIN      -2
#define  TF_BUSY       -3
#define  TF_DONE       -4
#define  TF_DECLINED   -5
#define  TF_ABORT      -6

#if (OS_LINUX)
#include <tf_config_linux.h>
#else

#define gettid() pthread_self()

#include <time.h>
#include <stdarg.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <sys/ddi.h>
#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>             /* offsetof() */
#include <stdint.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <glob.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sched.h>
#include <netinet/tcp.h>        /* TCP_NODELAY, TCP_CORK */
#include <netdb.h>
#include <sys/un.h>
#include <malloc.h>             /* memalign() */
#include <limits.h>             /* IOV_MAX */
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <crypt.h>
#include <sys/utsname.h>        /* uname() */

#endif

#define TF_INT32_LEN   sizeof("-2147483648") - 1
#define TF_INT64_LEN   sizeof("-9223372036854775808") - 1

#define LF     (u_char) 10
#define CR     (u_char) 13
#define CRLF   "\x0d\x0a"

#ifndef min
#define min(a, b)       ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b)       ((a) > (b) ? (a) : (b))
#endif

#define MAX_HOSTNAME_LEN   256
#define MAX_FILENAME_LEN   128
#define MAX_IOBUFFER_SIZE  4096
#define MAX_MQUEUE_LEN     4096

#if (__i386 || __amd64) && __GNUC__
#define GNUC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GNUC_VERSION >= 40100
#define HAVE_ATOMIC
#endif
#endif

#if defined(_SYS_FREEBSD_) || defined(_SYS_NETBSD_) || defined(_SYS_OPENBSD_)
#define nan(TC_a)      strtod("nan", NULL)
#define nanl(TC_a)     ((long double)strtod("nan", NULL))
#endif

__TF_CLINKAGEEND

#endif
