#include "tflib.h"
#include "tf_socket.h"
#include <fcntl.h>
#include <sys/stat.h>

#ifdef OS_SUNOS
int inet_aton (const char *cp, struct in_addr *inaddr)
{
  int dots = 0;  //2
  register u_long addr = 0;
  register u_long val = 0, base = 10;
  do
    {
      register char c = *cp;
      switch (c)
 {
 case '0': case '1': case '2': case '3': case '4': case '5':
 case '6': case '7': case '8': case '9':
   val = (val * base) + (c - '0');
   break;
 case '.':
   if (++dots > 3)
     return 0;
 case '\0':
   if (val > 255)
     return 0;
   addr = addr << 8 | val;
   val = 0;
   break;
 default:
   return 0;
 }
    } while (*cp++) ;
  if (dots < 3)
    addr <<= 8 * (3 - dots);
  if (inaddr)
    inaddr->s_addr = htonl (addr);
  return 1;
}
#endif

bool tfnet_nonblock(int fd) 
{
    int flags;

    /* Set the socket non-blocking.
     * Note that fcntl(2) for F_GETFL and F_SETFL can't be
     * interrupted by a signal. */
    if ((flags = fcntl(fd, F_GETFL)) == -1) {
        error(errno, "%s - fcntl(F_GETFL)", __func__);
        return false;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        error(errno, "%s fcntl(F_SETFL,O_NONBLOCK)", __func__);
        return false;
    }
    return true;
}


/* Set TCP keep alive option to detect dead peers. The interval option
 * is only used for Linux as we are using Linux-specific APIs to set
 * the probe send time, interval, and count. */
bool tfnet_keepalive(int fd, int interval)
{
    int val = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1) {
        error(errno, "%s - setsockopt SO_KEEPALIVE",  __func__);
        return false;
    }

#ifdef __linux__
    /* Default settings are more or less garbage, with the keepalive time
     * set to 7200 by default on Linux. Modify settings to make the feature
     * actually useful. */

    /* Send first probe after interval. */
    val = interval;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &val, sizeof(val)) < 0) {
        error(errno, "%s - setsockopt TCP_KEEPALIVE",  __func__);
        return false;
    }

    /* Send next probes after the specified interval. Note that we set the
     * delay as interval / 3, as we send three probes before detecting
     * an error (see the next setsockopt call). */
    val = interval/3;
    if (val == 0) val = 1;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, &val, sizeof(val)) < 0) {
        error(errno, "%s - setsockopt TCP_KEEPINTVL",  __func__);
        return false;
    }

    /* Consider the socket in error state after three we send three ACK
     * probes without getting a reply. */
    val = 3;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, &val, sizeof(val)) < 0) {
        error(errno, "%s - setsockopt TCP_KEEPNCT",  __func__);
        return false;
    }
#endif

    return true;
}

static bool tfnet_tcp_nodelay(int fd, int val)
{
    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1) {
        error(errno, "%s - setsockopt TCP_NODELAY", __func__ );
        return false;
    }
    return true;
}

bool tfnet_enable_tcp_nodelay(int fd)
{
    return tfnet_tcp_nodelay(fd, 1);
}

bool tfnet_disable_tcp_nodelay(int fd)
{
    return tfnet_tcp_nodelay(fd, 0);
}

bool tfnet_set_send_buffer(int fd, int buffsize)
{
    if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buffsize, sizeof(buffsize)) == -1) {
        error(errno, "%s - setsockopt SO_SNDBUF", __func__);
        return false;
    }
    return true;
}

bool tfnet_tcp_keepalive(int fd)
{
    int yes = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes)) == -1) {
        error(errno, "%s - setsockopt SO_KEEPALIVE", __func__);
        return false;
    }
    return true;
}

bool tfnet_resolve(char *host, char *ipbuf)
{
    struct sockaddr_in sa;

    sa.sin_family = AF_INET;
    if (inet_aton(host, &sa.sin_addr) == 0) {
        struct hostent *he;

        he = gethostbyname(host);
        if (he == NULL) {
            error(errno, "can't resolve: %s", host);
            return false;
        }
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }
    strcpy(ipbuf,inet_ntoa(sa.sin_addr));
    return true;
}

static int tfnet_create_socket(int domain) {
    int s, on = 1;
    if ((s = socket(domain, SOCK_STREAM, 0)) == -1) {
        error(errno, "%s - creating socket", __func__ );
        return TFNET_ERR;
    }

    /* Make sure connection-intensive things like the redis benchmark
     * will be able to close/open sockets a zillion of times */
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        error(errno, "%s - setsockopt SO_REUSEADDR", __func__);
        return TFNET_ERR;
    }
    return s;
}

#define TFNET_CONNECT_NONE     0
#define TFNET_CONNECT_NONBLOCK 1
static int tfnet_tcp_generic_connect(char *addr, int port, int flags)
{
    int s;
    struct sockaddr_in sa;

    if ((s = tfnet_create_socket(AF_INET)) == TFNET_ERR)
        return TFNET_ERR;

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    if (inet_aton(addr, &sa.sin_addr) == 0) {
        struct hostent *he;

        he = gethostbyname(addr);
        if (he == NULL) {
            error(errno, "can't resolve: %s", addr);
            close(s);
            return TFNET_ERR;
        }
        memcpy(&sa.sin_addr, he->h_addr, sizeof(struct in_addr));
    }

    if (flags & TFNET_CONNECT_NONBLOCK) {
        if (!tfnet_nonblock(s))
            return TFNET_ERR;
    }
    if (connect(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        if (errno == EINPROGRESS &&
            flags & TFNET_CONNECT_NONBLOCK)
            return s;

        error(errno, "connect server:%s error.", addr);
        close(s);
        return TFNET_ERR;
    }
    return s;
}

static int tfnet_tcp_generic_connect2(in_addr_t addr, int port, int flags)
{
    int s;
    struct sockaddr_in sa;

    if ((s = tfnet_create_socket(AF_INET)) == TFNET_ERR)
        return TFNET_ERR;

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(addr);

    if (flags & TFNET_CONNECT_NONBLOCK) {
        if (!tfnet_nonblock(s))
            return TFNET_ERR;
    }
    if (connect(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
        if (errno == EINPROGRESS &&
            flags & TFNET_CONNECT_NONBLOCK)
            return s;

        error(errno, "connect server:%s error.", addr);
        close(s);
        return TFNET_ERR;
    }
    return s;
}

int tfnet_tcp_connect(char *addr, int port)
{
    return tfnet_tcp_generic_connect(addr,port,TFNET_CONNECT_NONE);
}

int tfnet_tcp_connect2(in_addr_t addr, int port)
{
    return tfnet_tcp_generic_connect2(addr,port,TFNET_CONNECT_NONE);
}

int tfnet_tcp_nonblock_connect( char *addr, int port)
{
    return tfnet_tcp_generic_connect(addr,port,TFNET_CONNECT_NONBLOCK);
}

int tfnet_tcp_nonblock_connect2(in_addr_t addr, int port)
{
    return tfnet_tcp_generic_connect2(addr,port,TFNET_CONNECT_NONBLOCK);
}

static int tfnet_unix_generic_connect(char *path, int flags)
{
    int s;
    struct sockaddr_un sa;

    if ((s = tfnet_create_socket(AF_LOCAL)) == TFNET_ERR)
        return TFNET_ERR;

    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path,path,sizeof(sa.sun_path)-1);
    if (flags & TFNET_CONNECT_NONBLOCK) {
        if ( !tfnet_nonblock(s) )
            return TFNET_ERR;
    }
    if (connect(s,(struct sockaddr*)&sa,sizeof(sa)) == -1) {
        if (errno == EINPROGRESS &&
            flags & TFNET_CONNECT_NONBLOCK)
            return s;

        error(errno, "connect unix:%s error.", path);
        close(s);
        return TFNET_ERR;
    }
    return s;
}

int tfnet_unix_connect(char *path)
{
    return tfnet_unix_generic_connect(path,TFNET_CONNECT_NONE);
}

int tfnet_unix_nonblock_connect(char *path)
{
    return tfnet_unix_generic_connect(path,TFNET_CONNECT_NONBLOCK);
}

/* Like read(2) but make sure 'count' is read before to return
 * (unless error or EOF condition is encountered) */
int tfnet_read(int fd, char *buf, int count)
{
    int nread, loop, totlen = 0;
    loop = 0;
    while(totlen != count) {
        nread = read(fd,buf,count-totlen);
        if (nread == 0) return -1;
        if (nread == -1){
            if (errno == EAGAIN || errno == EINTR) {
                nread = 0;
                loop++;
                if(loop>20) return -1;
                continue;
            }
            return -1;
        }
        totlen += nread;
        buf    += nread;
    }
    return totlen;
}

/* Like write(2) but make sure 'count' is read before to return
 * (unless error is encountered) */
int tfnet_write(int fd, char *buf, int count)
{
    int nwritten, loop, totlen = 0;
    loop = 0;
    while(totlen != count) {
        nwritten = write(fd,buf,count-totlen);
        if (nwritten == 0) return -1;
        if (nwritten == -1){
            if (errno == EAGAIN || errno == EINTR) {
                nwritten = 0;
                loop++;
                if(loop>20) return -1;
                continue;
            }
            return -1;
        }
        totlen += nwritten;
        buf += nwritten;
    }
    return totlen;
}

static bool tfnet_listen(int s, struct sockaddr *sa, socklen_t len) {
    if (bind(s,sa,len) == -1) {
        error(errno, "bind socket error");
        close(s);
        return false;
    }

    /* Use a backlog of 512 entries. We pass 511 to the listen() call because
     * the kernel does: backlogsize = roundup_pow_of_two(backlogsize + 1);
     * which will thus give us a backlog of 512 entries */
    if (listen(s, 511) == -1) {
        error(errno, "listen socket error");
        close(s);
        return false;
    }
    return true;
}

int tfnet_tcp_server(int port, char *bindaddr)
{
    int s;
    struct sockaddr_in sa;

    if ((s = tfnet_create_socket(AF_INET)) == TFNET_ERR)
        return TFNET_ERR;

    memset(&sa,0,sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bindaddr && inet_aton(bindaddr, &sa.sin_addr) == 0) {
        error(errno, "invalid bind address");
        close(s);
        return TFNET_ERR;
    }
    if (!tfnet_listen(s,(struct sockaddr*)&sa,sizeof(sa)))
        return TFNET_ERR;
    return s;
}

int tfnet_tcp_server2(int port, in_addr_t ip)
{
    int s;
    struct sockaddr_in sa;

    if ((s = tfnet_create_socket(AF_INET)) == TFNET_ERR)
        return TFNET_ERR;

    memset(&sa,0,sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port        = htons(port);
    sa.sin_addr.s_addr = htonl(ip);

    if (!tfnet_listen(s,(struct sockaddr*)&sa,sizeof(sa)))
        return TFNET_ERR;
    return s;
}

int tfnet_unix_server(char *path, mode_t perm)
{
    int s;
    struct sockaddr_un sa;

    if ((s = tfnet_create_socket(AF_LOCAL)) == TFNET_ERR)
        return TFNET_ERR;

    memset(&sa,0,sizeof(sa));
    sa.sun_family = AF_LOCAL;
    strncpy(sa.sun_path,path,sizeof(sa.sun_path)-1);
    if (!tfnet_listen(s,(struct sockaddr*)&sa,sizeof(sa)))
        return TFNET_ERR;
    if (perm) chmod(sa.sun_path, perm);
    return s;
}

static int tfnet_generic_accept(int s, struct sockaddr *sa, socklen_t *len) {
    int fd;
    while(1) {
        fd = accept(s,sa,len);
        if (fd == -1) {
            if (errno == EINTR)
                continue;
            else {
                error(errno, "accept error");
                return TFNET_ERR;
            }
        }
        break;
    }
    return fd;
}

int tfnet_tcp_accept(int s, char *ip, int *port) {
    int fd;
    struct sockaddr_in sa;
    socklen_t salen = sizeof(sa);
    if ((fd = tfnet_generic_accept(s,(struct sockaddr*)&sa,&salen)) == TFNET_ERR)
        return TFNET_ERR;

    if (ip) strcpy(ip,inet_ntoa(sa.sin_addr));
    if (port) *port = ntohs(sa.sin_port);
    return fd;
}

int tfnet_tcp_accept2(int s, in_addr_t *ip, int *port) {
    int fd;
    struct sockaddr_in sa;
    socklen_t salen = sizeof(sa);
    if ((fd = tfnet_generic_accept(s,(struct sockaddr*)&sa,&salen)) == TFNET_ERR)
        return TFNET_ERR;

    if (ip)     *ip = ntohl(sa.sin_addr.s_addr);
    if (port) *port = ntohs(sa.sin_port);
    return fd;
}

int tfnet_unix_accept(int s) {
    int fd;
    struct sockaddr_un sa;
    socklen_t salen = sizeof(sa);
    if ((fd = tfnet_generic_accept(s,(struct sockaddr*)&sa,&salen)) == TFNET_ERR)
        return TFNET_ERR;

    return fd;
}

bool tfnet_sock_peer(int fd, char *ip, int *port) {
    struct sockaddr_in sa;
    socklen_t salen = sizeof(sa);

    if (getpeername(fd,(struct sockaddr*)&sa,&salen) == -1) {
        *port = 0;
        ip[0] = '?';
        ip[1] = '\0';
        return false;
    }
    if (ip) strcpy(ip,inet_ntoa(sa.sin_addr));
    if (port) *port = ntohs(sa.sin_port);
    return true;
}

in_addr_t tfnet_pton(int af, const char *src)
{
    struct in_addr s;
    if(inet_pton(af, src, &s)<0){
        error(0,"inet_pton convert error:%s", src);
        return 0;
    }

    return ntohl(s.s_addr);
}

char *tfnet_ntop(int af, in_addr_t addr)
{
    static char ip[20];
    struct in_addr s;
    s.s_addr = htonl(addr);
    if(inet_ntop(af, &s, ip, (socklen_t )sizeof(ip))==NULL){
        error(0,"inet_ntop convert error:%u", addr);
        return NULL;
    }
    return ip; 
}


void net16cpy(unsigned short *dst,const char *buf)
{
    memcpy(dst, buf, 2);
    *dst = ntohs(*dst);
}

void net32cpy(unsigned int *dst,const char *buf)
{
    memcpy(dst, buf, 4);
    *dst = ntohl(*dst);
}

void host16cpy(char *buf, unsigned short src)
{
    unsigned short u16 = htons(src);
    memcpy(buf, &u16, 2);
}

void host32cpy(char *buf, unsigned int src)
{
    unsigned int u32 = htonl(src);
    memcpy(buf, &u32, 4);
}
