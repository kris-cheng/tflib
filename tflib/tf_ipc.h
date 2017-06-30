#ifndef _TF_IPC_H_
#define _TF_IPC_H_

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

/* message queue */
typedef struct mq_data_st *mq_data_t;
typedef struct mq_data_st
{
    long  msg_type;
    char  data[MAX_MQUEUE_LEN];
}MQ_DATA_ST;

int    ipc_queue_create(const char *filename, int id);
bool   ipc_queue_delete(int id);
size_t ipc_queue_size();
bool   ipc_write_queue(int id, mq_data_t,int len);

/* share memory */
void *ipc_shm_create(const char *filename, int id, size_t size);
void *ipc_shm_open(const char *filename, int id);

#endif
