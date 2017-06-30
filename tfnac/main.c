#define VERSION_NUM  "1.0.0"
#define VERSION_DATE __DATE__

#include <main.h>
#include <netinet/in.h>
#include "funcs.h"

process_mgr_t process_mgr;

void safe_quit();
void peer_request_handler(aeEventLoop *el, int fd, void *data, int mask);

peer_info_t get_idle_peer();
peer_info_t get_active_peer(__u32 id);
void close_peer(peer_info_t peer);

static void signal_handler(int signum)
{
    switch (signum)
    {
        case SIGINT:
        case SIGTERM:
            if(!process_mgr->quitting) {
                info(0,"SIGTERM quitting... ...");
                process_mgr->quitting = true;
                safe_quit();
            }
            break;
        case SIGHUP:
            break;
        case SIGQUIT:
            if(!process_mgr->quitting) {
                info(0,"SIGQUIT quitting... ...");
                process_mgr->quitting = true;
                safe_quit();
            }
            break;
        case SIGPIPE:
            break;
        case SIGUSR1:
            if(process_mgr->log_level<LOG_LEVEL_NUMS) {
                process_mgr->log_level = process_mgr->log_level+1;
                log_set_level(process_mgr->log_level);
            }else {
                process_mgr->log_level = 0;
                log_set_level(process_mgr->log_level);
            }
            info(0,"Set log level to %d",process_mgr->log_level);
            break;
        case SIGUSR2:
            if(process_mgr->log_level>0) {
                process_mgr->log_level = process_mgr->log_level-1;
                log_set_level(process_mgr->log_level);
            }else {
                process_mgr->log_level = 4;
                log_set_level(process_mgr->log_level);
            }
            info(0,"Set log level to %d",process_mgr->log_level);
            break;
    }
}

static void setup_signal_handlers(void)
{
    struct sigaction act;

    act.sa_handler = signal_handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT,  &act, NULL);
    sigaction(SIGTERM, &act, NULL);
    sigaction(SIGQUIT, &act, NULL);
    sigaction(SIGHUP,  &act, NULL);
    sigaction(SIGPIPE, &act, NULL);
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
}

void safe_quit()
{
    aeStop(process_mgr->el);

    if(!process_mgr->mq_exit){
        memcpy(&process_mgr->mq_data.data[0], "0000", 4);
        process_mgr->mq_data.msg_type = process_mgr->app_id;
        ipc_write_queue(process_mgr->device_in_queue_id, &process_mgr->mq_data, 4);
    }
}

peer_info_t get_idle_peer() {
    int i = 0;
    for( i = 0;i<MAX_PEER_NUMS;i++){
        if(!process_mgr->_s_->slots[i].active )
            return &(process_mgr->_s_->slots[i]);
    }
    return NULL;
}

peer_info_t get_active_peer(__u32 id) {
    peer_info_t peer;
    int index = id - PEER_ID_OFFSET;

    if(index>0 && index<MAX_PEER_NUMS){
        peer = &process_mgr->_s_->slots[index];
        if(peer->active){
            return peer;
        }
    }

    return NULL;
}

void close_peer(peer_info_t peer){
    if(!peer) return;

    aeDeleteFileEvent(process_mgr->el,peer->fd,AE_READABLE);

    peer->active = false;
    peer->authed = false;

    close(peer->fd);
    process_mgr->connections--;

    debug(0,"Peer:%u closed", peer->id);
}

int life_cycle_handler(struct aeEventLoop *el, long long id, void *_process){
    int i = 0;
    peer_info_t peer;

    for( i = 0;i<MAX_PEER_NUMS;i++){
        peer = &process_mgr->_s_->slots[i];
        if(peer->active){
            peer->session_timer--;    
            if(peer->session_timer == 0){
                info(0,"Close timeout peer[%u], socket[%s:%u]", peer->id, tfnet_ntop(AF_INET, peer->ip), peer->port);
                close_peer(peer);
            }
        }
    }
    return process_mgr->cycle_step;
}

void accept_tcp_handler(aeEventLoop *el, int fd, void *data, int mask){
    int cport, cfd, cip;

    AE_NOTUSED(el);
    AE_NOTUSED(mask);

    cfd = tfnet_tcp_accept2(fd, &cip, &cport);
    if (cfd == TFNET_ERR) {
        alert(errno,"Accepting client connection error");
        return;
    }

    peer_info_t peer = get_idle_peer();
    if(peer == NULL) {
        error(0,"No enought link resource for connect");
        return;
    }

    tfnet_nonblock(cfd);
    tfnet_enable_tcp_nodelay(cfd);

    peer->fd    = cfd;
    peer->ip    = cip;
    peer->port  = cport;

    peer->active = true;

    process_mgr->connections++;

    if (aeCreateFileEvent(process_mgr->el, cfd, AE_READABLE, peer_request_handler, peer) == AE_ERR) {
        close_peer(peer);
        error(0,"Create client file event error!");
        return;
    }

    peer->session_timer = process_mgr->socket_timeout;
    debug(0,"Accepted %s:%d Total peers:%d fd:%d", tfnet_ntop(AF_INET, cip), cport, process_mgr->connections, cfd);
}

void peer_request_handler(aeEventLoop *el, int fd, void *data, int mask){
    peer_info_t    peer    = (peer_info_t)data;
    if(!peer) return;

    __u16 len;

    if (tfnet_read(fd, (char *)&len, 2) != 2){
        error(errno, "Read socket broken!Reset peer[%u] connection...", peer->id);
        close_peer(peer);
        return;
    }

    len = ntohs(len);
    if(len>MAX_PACKET_LEN || len<8){
        error(0,"Socket data len out of range:%d close peer:%u",len, peer->id);
        close_peer(peer);
        return;
    }

    if (tfnet_read(fd, (char *)process_mgr->buffer, len) != len){
        error(errno, "Read socket broken!Reset peer[%u] connection...", peer->id);
        close_peer(peer);
        return;
    }

    __u32 dstid, cmd, wlen;

    net32cpy(&cmd, &process_mgr->buffer[0]);
    dstid = (0x0fff0000 & cmd) >> 16;

    wlen = 0;
    host32cpy( &process_mgr->mq_data.data[ 0], process_mgr->app_id); wlen += 4;
    host32cpy( &process_mgr->mq_data.data[ 4], dstid); wlen += 4;
    host32cpy( &process_mgr->mq_data.data[ 8], peer->id); wlen += 4;
    host32cpy( &process_mgr->mq_data.data[12], time(0)); wlen += 4;
    host32cpy( &process_mgr->mq_data.data[16], cmd); wlen += 4;
    host32cpy( &process_mgr->mq_data.data[20], 0);   wlen += 4;
    memcpy(&process_mgr->mq_data.data[24],&process_mgr->buffer[8],len - 8);  wlen = wlen + len - 8;

    process_mgr->mq_data.msg_type = dstid;
    if(ipc_write_queue(process_mgr->device_out_queue_id, &process_mgr->mq_data, wlen)){
        peer->session_timer = process_mgr->socket_timeout;
    }else{
        close_peer(peer);
    }
}

void message_response_handler(aeEventLoop *el, int fd, void *data, int mask){
    __u16 len, wlen;
    __u32 id;
    if (tfnet_read(fd, (char *)&len, 2) != 2){
        error(errno, "Error read socketpare data len");
        return;
    }

    len = ntohs(len);
    if (tfnet_read(fd, (char *)&process_mgr->buffer, len) != len){
        error(errno, "Error read socketpare data");
        return;
    }

    net32cpy(&id, &process_mgr->buffer[8]);
    peer_info_t peer = (peer_info_t)get_active_peer(id);
    if ( peer == NULL) {
        error(0,"Peer client[%u] info lost", id);
        return;
    }

    wlen = len - 16;
    host16cpy(&process_mgr->buffer[14], wlen);

    if (tfnet_write( peer->fd, &process_mgr->buffer[14], wlen+2) < 0){
        close_peer(peer);
    }else{
        peer->session_timer = process_mgr->socket_timeout;
    }
}

tf_thread_value_t wait_response_thread(void *arg){
    int  len;
    __u32 id, srcid, dstid;
    __u16 wlen;
    MQ_DATA_ST mq_data;

    debug(0,"Message read handle thread started.");
    process_mgr->mq_exit = false;

    while(!process_mgr->quitting){
        len = msgrcv( process_mgr->device_in_queue_id, &mq_data, MAX_MQUEUE_LEN, process_mgr->app_id, 0);
        if( len == 4 ) {
           debug(0,"Safe quit message received!");
           continue;
        }

        if( len < 24 ) {
           error(errno,"Error msgrcv[%s:%d] len:%d",__func__,__LINE__, len);
           continue;
        }

        wlen = len;
        wlen = htons(wlen);

        net32cpy(&id, &mq_data.data[8]);
        net32cpy(&srcid, &mq_data.data[16]);
        net32cpy(&dstid, &mq_data.data[20]);

        debug(0,"Response data for[%u] src[%08x] dst[%08x]", id, srcid, dstid); 
        tfnet_write(process_mgr->mqueue[1], (void *)&wlen, 2);
        tfnet_write(process_mgr->mqueue[1], mq_data.data, len);
    }

    debug(0,"Message read handle thread stoped.");
    process_mgr->mq_exit = true;

    return (tf_thread_value_t) true;
}

int main(int argc, char *argv[])
{
    setup_signal_handlers();

    process_mgr = (process_mgr_t)tf_alloc(sizeof(PROCESS_MGR_ST));
    memset(process_mgr, 0, sizeof(PROCESS_MGR_ST));

    bool isconfig,isconsole;
    int argn = 1;
    isconfig = isconsole = false;
    while ( argn < argc && argv[argn][0] == '-' && argv[argn][1] != '\0' ) {
        if(strcmp(argv[argn],"-v") == 0 ) {
            info(0,"%s Version:%s Public date:%s",argv[0], VERSION_NUM,VERSION_DATE);
        }else if(strcmp(argv[argn],"-f") == 0 && argn + 1 < argc ) {
            strcpy(process_mgr->conf_file, argv[++argn]);
            isconfig = true;
        }else if(strcmp(argv[argn],"-cli")==0) {
            isconsole = true;
        }
        argn++;
    }

    if(!isconfig){
        info(0,"usage:%s -f configfile", argv[0]);
        tf_free(process_mgr);
        exit(1);
    }

    if(!initialize(process_mgr->conf_file, isconsole)){
        emerg(errno,"Config file initialized error!");
        tf_free(process_mgr);
        exit(1);
    }

    if(isconsole){
        run_console();
    }else{
        adjust_peers_limit();

        if (process_mgr->server_port != 0) {
            process_mgr->server_fd = tfnet_tcp_server2(process_mgr->server_port, process_mgr->server_ip);
            if(process_mgr->server_fd == TFNET_ERR ) {
                error(errno, "Create listen socket error! [%s:%u]",tfnet_ntop(AF_INET, process_mgr->server_ip), process_mgr->server_port);
                process_mgr->quitting = true;
            }else{
                process_mgr->el      = aeCreateEventLoop(MAX_PEER_NUMS+1024);
                if(process_mgr->el == NULL){
                    error(0, "Create main event loop error!");
                    process_mgr->quitting = true;
                }else{
                    if (aeCreateFileEvent(process_mgr->el,process_mgr->server_fd, AE_READABLE, accept_tcp_handler, process_mgr) == AE_ERR)  {
                        error(0, "Unrecoverable error creating socket server file event.");
                        aeDeleteEventLoop(process_mgr->el);
                        close(process_mgr->server_fd);
                        process_mgr->quitting = true;
                    }
                }
            }
        }

        if(!process_mgr->quitting){
            if ((socketpair(AF_LOCAL, SOCK_STREAM, 0, process_mgr->mqueue)) == -1) {
                error(0, "Create parent process mqueue socketpair error");
                process_mgr->quitting = true;
            }else{
                if(aeCreateFileEvent(process_mgr->el, process_mgr->mqueue[0], AE_READABLE, message_response_handler, process_mgr) == AE_ERR)  {
                    error(0, "Unrecoverable error creating message queue file event.");
                    process_mgr->quitting = true;
                }else{
                    process_mgr->cycle_timer_id = aeCreateTimeEvent(process_mgr->el, process_mgr->cycle_step, life_cycle_handler, process_mgr, NULL);

                    info(0,"Main process message queue read event,fd[%d]", process_mgr->mqueue[1]);

                    tf_create_thread(&process_mgr->mq_thread, wait_response_thread, (void *)process_mgr);
   
                    aeMain(process_mgr->el);
                }

                close(process_mgr->mqueue[0]);
                close(process_mgr->mqueue[1]);

                close(process_mgr->server_fd);

                info(0,"Process exit");
            }

            aeDeleteEventLoop(process_mgr->el);
        }
    }

    cleanup();
    tf_free(process_mgr);
    exit(0);
}
