#include "main.h"

extern process_mgr_t process_mgr;

void init_peers(){
    if(!process_mgr->reload) return;

    process_mgr->_s_->current = 0;
    for(int i=0;i<MAX_PEER_NUMS;i++){
        process_mgr->_s_->slots[i].id    = i + PEER_ID_OFFSET;
        process_mgr->_s_->slots[i].fd    = 0;
        process_mgr->_s_->slots[i].active = false;
        process_mgr->_s_->slots[i].authed = false;
        process_mgr->_s_->slots[i].session_timer = 0;
    }

    info(0, "%d peers initilized OK!", MAX_PEER_NUMS);
}

bool load_sharememory(){
    process_mgr->_s_ = (shmem_mgr_t)ipc_shm_open(process_mgr->app_share_path, process_mgr->app_memory_id);

    if(process_mgr->_s_ == NULL){
        process_mgr->_s_ = (shmem_mgr_t)ipc_shm_create(process_mgr->app_share_path, process_mgr->app_memory_id , sizeof(SHMEM_MGR_ST));
        if(process_mgr->_s_ != NULL){
            memset(process_mgr->_s_, 0, sizeof(SHMEM_MGR_ST));
            process_mgr->reload = true;
        }
    }

    if(process_mgr->_s_ == NULL ) return false;

    return true;
}

bool create_messagequeue(){
    process_mgr->device_in_queue_id   = ipc_queue_create(process_mgr->device_queue_path, process_mgr->device_in_msg_id);
    process_mgr->device_out_queue_id  = ipc_queue_create(process_mgr->device_queue_path, process_mgr->device_out_msg_id);

    if(process_mgr->device_in_queue_id<0 || process_mgr->device_out_queue_id<0){
        return false;
    }

    return true;
}

bool initialize(const char *filename,bool console){
    cfg_file_t cfg = cfg_create("%s/cfg/%s",getenv("HOME"),filename);
    if(cfg == NULL) return false;

    cfg_group_t group = cfg_find_group(cfg, "general");
    if(group != NULL){
        process_mgr->log_level = cfg_get_int(group,"log_file_level",DEBUG);
        strcpy(process_mgr->log_file, cfg_get_string(group, "log_file_name", "/tmp/nac-service.log"));
        /* log file && log level*/

        process_mgr->app_id     = cfg_get_int(group, "app_id",    120);
        process_mgr->device_id  = cfg_get_int(group, "device_id", 1200);
        process_mgr->reload = cfg_get_bool(group, "reload", false);

        strcpy(process_mgr->app_share_path, cfg_get_string(group, "app_share_path", "/opt/kris/etc/memory.ipc"));
        process_mgr->app_memory_id = cfg_get_int(group, "app_memory_id", 1);

        strcpy(process_mgr->device_queue_path, cfg_get_string(group, "device_queue_path", "/opt/kris/etc/msgqueue.ipc"));
        process_mgr->device_in_msg_id   = cfg_get_int(group, "device_in_msg_id",   15);
        process_mgr->device_out_msg_id = cfg_get_int(group, "device_out_msg_id", 16);

        process_mgr->server_ip   = tfnet_pton(AF_INET, cfg_get_string(group, "server_ip", "127.0.0.1"));
        process_mgr->server_port = cfg_get_int(group, "server_port", 7960);
        process_mgr->socket_timeout  = cfg_get_int(group, "socket_timeout",  300);
        process_mgr->cycle_step  = cfg_get_int(group, "cycle_step",  1000);

        process_mgr->server_fd   = 0;
        process_mgr->connections = 0;

        process_mgr->device_in_queue_id   = -1;
        process_mgr->device_out_queue_id = -1;
    }else{
        cfg_destroy(cfg);
        return false;
    }

    if (!load_sharememory()) {
        cfg_destroy(cfg);
        return false;
    }
    debug(0,"Load share memory ok!");

    if (!create_messagequeue()){
        cfg_destroy(cfg);
        return false;
    }
    debug(0,"Create message queue ok!");

    if(!console){
        init_peers();
    }
    log_create(process_mgr->log_level, process_mgr->log_file);

    cfg_destroy(cfg);
    return true;
}

void cleanup() {
    info(0, "Destory log file");
    log_destroy();
}

void adjust_peers_limit(void)
{
    int max_nums;
    rlim_t maxfiles = MAX_PEER_NUMS + 32;
    struct rlimit limit;

    if (getrlimit(RLIMIT_NOFILE,&limit) == -1) {
        error(0,"Unable to obtain the current NOFILE limit (%s), assuming 1024 and setting the max clients configuration accordingly.",
            strerror(errno));
        max_nums = 1024-32;
    } else {
        rlim_t oldlimit = limit.rlim_cur;

        /* Set the max number of files if the current limit is not enough for our needs. */
        if (oldlimit < maxfiles) {
            rlim_t f;

            f = maxfiles;
            while(f > oldlimit) {
                limit.rlim_cur = f;
                limit.rlim_max = f;
                if (setrlimit(RLIMIT_NOFILE,&limit) != -1) break;
                f -= 128;
            }
            if (f < oldlimit) f = oldlimit;
            if (f != maxfiles) {
                max_nums = f-32;
                error(0,"Unable to set the max number of files limit to %d (%s), setting the max clients configuration to %d.",
                    (int) maxfiles, strerror(errno), (int) max_nums);
            } else {
                info(0,"Max number of open files set to %d", (int) maxfiles);
            }
        }
    }
}
