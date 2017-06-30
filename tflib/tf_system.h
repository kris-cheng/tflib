#ifndef _TF_SYSTEM_H_
#define _TF_SYSTEM_H_

typedef struct tf_cycle_st *tf_cycle_t;
typedef struct tf_cycle_st
{
    bool daemon;
    bool master;

    __u8  conf_file[MAX_FILENAME_LEN];
    __u8  log_file[MAX_FILENAME_LEN];
    __u8  log_level;
}TF_CYCLE_ST;

tf_cycle_t tf_cycle_new();
bool tf_cycle_init(tf_cycle_t cycle);
void tf_cycle_destroy(tf_cycle_t cycle);

void create_pid_file(const char *fliename);
void daemonize(void);
void setaffinity(unsigned int cpu);

#endif
