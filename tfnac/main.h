#ifndef _MAIN_H_
#define _MAIN_H_

#include <tflib.h>
#include <ae/ae.h>

#define MAX_PEER_NUMS       10000
#define PEER_ID_OFFSET      20140000
#define MAX_PACKET_LEN      4096

///////////////////////////////////////////////////////////////

typedef struct peer_info_st     *peer_info_t;
typedef struct process_mgr_st   *process_mgr_t;
typedef struct shmem_mgr_st     *shmem_mgr_t;

#pragma pack(1)

typedef struct peer_info_st{
    __u32 id;
    int   fd;

    __u8  active:1;
    __u8  authed:1;
    __u8  reserved:6;

    __u32 ip;
    __u16 port;

    __u16 session_timer;
}PEER_INFO_ST;

typedef struct shmem_mgr_st{
    __u16           current;
    PEER_INFO_ST    slots[MAX_PEER_NUMS];
}SHMEM_MGR_ST;

#pragma pack(0)

///////////////////////////////////////////////////////////////
typedef struct process_mgr_st{
    __u8  conf_file[MAX_FILENAME_LEN];
    __u8  log_file[MAX_FILENAME_LEN];
    __u8  log_level;

    bool  reload;

    int   app_id;
    int   device_id;

      int server_fd;
    __u32 server_ip;
    __u16 server_port;
    __u16 socket_timeout;
    __u16 connections;

    __u32         cycle_step;
    long long     cycle_timer_id;

    __u8  app_share_path[MAX_FILENAME_LEN];
    int   app_memory_id;

    __u8  device_queue_path[MAX_FILENAME_LEN];
    int   device_in_msg_id;
    int   device_in_queue_id;
    int   device_out_msg_id;
    int   device_out_queue_id;

    aeEventLoop  *el;
    int    mqueue[2];

    pthread_t  mq_thread;
    bool       mq_exit;
    bool       quitting;
    shmem_mgr_t         _s_;

    MQ_DATA_ST          mq_data;
    char                data[MAX_MQUEUE_LEN];
    char                buffer[MAX_MQUEUE_LEN];
}PROCESS_MGR_ST;

#endif
