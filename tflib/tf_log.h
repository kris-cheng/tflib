#ifndef _TF_LOG_H_
#define _TF_LOG_H_

#define TF_MAX_ERROR_STR   2048

typedef struct tf_log_st *tf_log_t;

typedef enum
{
    DEBUG=0, INFO=1, NOTICE=2, WARN=3, ERROR=4, CRIT=5, ALERT=6, EMERG=7, LOG_LEVEL_NUMS=8
}LOG_LEVEL_EN;

struct tf_log_st{
    __u8           level;   
    bool           opened;
    FILE          *file;
};

void log_create(__u8 level,const char *name);
void log_append(__u8 level,const char *name);
void log_destroy(void);
void log_set_level(__u8 level);

static inline void log_stderr(char *text) {
    write(STDERR_FILENO, text, strlen(text));
    write(STDERR_FILENO, "\n", 1);
}

void debug(int err, const char *fmt, ...);
void info(int err, const char *fmt, ...);
void notice(int err, const char *fmt, ...);
void warn(int err, const char *fmt, ...);
void error(int err, const char *fmt, ...);
void crit(int err, const char *fmt, ...);
void alert(int err, const char *fmt, ...);
void emerg(int err, const char *fmt, ...);

#define assert(TF_expr) \
  do { \
    if(!(TF_expr)){ \
      error(0, "assertion failed: %s", #TF_expr); \
      abort(); \
    } \
  } while(false)

void dump(const char *data, int len);

#endif /* _TF_LOG_H_ */
