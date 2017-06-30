#include "tflib.h"
#include "tf_ipc.h"

static key_t  ipc_unique_key(const char *filename, int id ){
    key_t       keyv;

    if ( access( filename, R_OK ) < 0 ) {
        error(0,"KEY file[%s] cann't access.", filename);
        return -1;
    }

    keyv=ftok(filename,id);
    if( keyv == -1) {
        error(0,"Create key tok error");
        return -1;
    }

    return keyv;
}

int ipc_queue_create(const char *filename, int id){
    key_t       key = ipc_unique_key(filename, id);
    if(key < 0) {
        return -1;
    }

    int msgid;
    msgid = msgget(key, IPC_CREAT|0666);
    if (msgid < 0) {
        error(errno, "Create message queue failed.[%s][%d]", filename, id);
        return -1;
    }
    return msgid;
}

bool ipc_queue_delete(int msgid){
    if (msgid < 0)
        return false;

    if (msgctl(msgid, IPC_RMID, NULL) < 0)
        return false;

    return true;
}

size_t ipc_queue_size(int msgid){
    struct msqid_ds info;
    if (msgctl(msgid, IPC_STAT, &info) < 0)
        return 0;

    return info.msg_qnum;
}

bool ipc_write_queue(int id, mq_data_t pkg,int len){
    if(len>=MAX_MQUEUE_LEN || len<= 0) {
        error(0,"Error message len:%d, out of limit",len);
        return false;
    }

    if (id < 0 || pkg->msg_type == 0) {
        error(errno, "Message queue write failed, queue id or msg type error[%d-%d]", id, pkg->msg_type);
        return false;
    }

    if (msgsnd(id, pkg, len, IPC_NOWAIT) < 0) {
        error(errno, "(%d): Write to queue(id:%08x type:%08x) failed", getpid(), id,pkg->msg_type);
        return false; 
    }

    return true;
}

void *ipc_shm_create(const char *filename, int id, size_t size){
    key_t       key = ipc_unique_key(filename, id);
    if(key < 0) {
        return NULL;
    }

    int shmid;
    void *ptr = NULL;

    shmid = shmget(key, size, IPC_CREAT|IPC_EXCL|SHM_R|SHM_W);
    if (shmid < 0) {
        error(errno, "Create share memory failed.");
        return NULL;
    }

    ptr = (void *)shmat(shmid, 0, 0);
    if (ptr == NULL) {
        error(errno, "Attach share memory failed.");
        return NULL;
    }
    return ptr;
}

void *ipc_shm_open(const char *filename, int id){
    key_t       key = ipc_unique_key(filename, id);
    if(key < 0) {
        return NULL;
    }

    int shmid;
    void *ptr = NULL;

    shmid = shmget(key, 0, 0);
    if (shmid < 0) {
        error(errno, "Open share memory failed.");
        return NULL;
    }

    ptr = (void *)shmat(shmid, 0, 0);
    if (ptr == NULL) {
        error(errno, "Attach share memory failed.");
        return NULL;
    }
    return ptr;
}
