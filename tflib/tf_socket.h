#ifndef _TF_SOCKET_H_
#define _TF_SOCKET_H_

#define TFNET_OK       0
#define TFNET_ERR     -1

#if defined(__sun)
#define AF_LOCAL AF_UNIX
#endif

/* unix domain socket */
int tfnet_unix_connect(char *path);
int tfnet_unix_nonblock_connect(char *path);
int tfnet_unix_server(char *path, mode_t perm);
int tfnet_unix_accept(int serversock);

/* common socket function*/
int tfnet_read(int fd, char *buf, int count);
int tfnet_write(int fd, char *buf, int count);
bool tfnet_nonblock(int fd);
bool tfnet_resolve(char *host, char *ipbuf);
bool tfnet_sock_peer(int fd, char *ip, int *port);
bool tfnet_keepalive(int fd, int interval);
bool tfnet_set_send_buffer(int fd, int buffsize);
in_addr_t tfnet_pton(int af, const char *src);
char *tfnet_ntop(int af, in_addr_t addr);

void net16cpy(unsigned short *dst,const char *buf);
void net32cpy(unsigned int *dst,const char *buf);
void host16cpy(char *buf, unsigned short src);
void host32cpy(char *buf, unsigned int src);

/* tcp socket */
int tfnet_tcp_connect(char *addr, int port);
int tfnet_tcp_connect2(in_addr_t addr, int port);
int tfnet_tcp_nonblock_connect(char *addr, int port);
int tfnet_tcp_nonblock_connect2(in_addr_t addr, int port);
int tfnet_tcp_server(int port, char *bindaddr);
int tfnet_tcp_server2(int port, in_addr_t ip);
int tfnet_tcp_accept(int serversock, char *ip, int *port);
int tfnet_tcp_accept2(int serversock, in_addr_t *ip, int *port);
bool tfnet_enable_tcp_nodelay(int fd);
bool tfnet_disable_tcp_nodelay(int fd);
bool tfnet_tcp_keepalive(int fd);

/* unix socketpair */

#endif /* _TF_SOCKET_H_ */
