#include "tflib.h"
#include "tf_log.h"
#include <ctype.h>

static struct tf_log_st   tf_log;
static const char *err_levels[] = {"[DBG]", "[INF]", "[NTC]", "[WAR]", "[ERR]", "[CRT]", "[ALT]", "[EMG]" };
volatile char tf_log_time[] ="MMDD-HHMISS";

void log_create(__u8 level, const char *name) {
    tf_log.level = level;

    tf_log.file = fopen(name, "a");
    if( tf_log.file == NULL ){
        error(errno,  "Couldn't open logfile `%s'.", name);
        tf_log.opened = false;
        return;
    }
    tf_log.opened = true;
    tf_datetime_mmddhhmiss((char *)tf_log_time);
}

void log_append(__u8 level, const char *name) {
    if(tf_log.opened){
        log_destroy();
    }
    log_create(level, name);
}

void log_destroy(void){
    if(tf_log.opened && tf_log.file != NULL){
        fflush(tf_log.file);
        fclose(tf_log.file);
        tf_log.file = NULL;
        tf_log.opened = false;
    }
}

void log_set_level(__u8 level){
    tf_log.level = level;
}

#define log_write(error_no, log_level) { \
        va_list args; \
        char outbuf[TF_MAX_ERROR_STR];\
        if (log_level < 0 || log_level >= LOG_LEVEL_NUMS) \
            return; \
        tf_datetime_mmddhhmiss((char *)tf_log_time);\
        sprintf(outbuf, "%s[%s-%03d] ", \
            err_levels[log_level], tf_log_time, gettid() % 1000); \
        va_start(args, fmt); \
        vsprintf(outbuf+strlen(outbuf), fmt, args); \
        va_end(args); \
        if (error_no > 0) {\
            strcat(outbuf, ":");\
            strcat(outbuf, (char *)strerror(error_no));\
        }\
        if(tf_log.opened) { \
            if (tf_log.file != NULL && log_level >= tf_log.level) { \
                fprintf(tf_log.file, "%s\n", outbuf);\
            } \
        } \
        log_stderr(outbuf);\
    }

void debug(int err, const char *fmt, ...) {
    log_write(err, DEBUG);
}

void info(int err, const char *fmt, ...) {
    log_write(err, INFO);
}

void notice(int err, const char *fmt, ...) {
    log_write(err, NOTICE);
}

void warn(int err, const char *fmt, ...) {
    log_write(err, WARN);
}

void error(int err, const char *fmt, ...) {
    log_write(err, ERROR);
}

void crit(int err, const char *fmt, ...) {
    log_write(err, CRIT);
}

void alert(int err, const char *fmt, ...) {
    log_write(err, ALERT);
}

void emerg(int err, const char *fmt, ...) {
    log_write(err, EMERG);
}

void dump(const char *data, int len){
    int  c,i = 0;
    char buf[TF_MAX_ERROR_STR*4],buf2[TF_MAX_ERROR_STR];
    memset(buf,0,sizeof(buf));
    memset(buf2,0,sizeof(buf2));
    unsigned char  B;

    if(len>0)
    {
         c = B = data[0];
         sprintf(buf," %02X",B);
         if(isprint(c))
             sprintf(buf2,"%c",(data[0]));
         else
             sprintf(buf2,".");
    }
    for(i=1;i<len;i++)
    {
        c = B = data[i];
        sprintf(buf,"%s %02X",buf,B);
        if(isprint(c))
            sprintf(buf2,"%s%c",buf2,B);
        else
            sprintf(buf2,"%s.",buf2);
        if(((i+1)%8 )== 0)
        {
            sprintf(buf,"%s ",buf);
            sprintf(buf2,"%s ",buf2);
        }
        if(((i+1)%16 )== 0)
        {
             sprintf(buf,"%s %s\n",buf,buf2);
             memset(buf2,0,sizeof(buf2));
        }
    }
    i = strlen(buf2);
    if(i>0)
    {
        if(i>8) i--;
        sprintf(buf,"%s ",buf);
        while((i%16)!=0)
        {
            sprintf(buf,"%s   ",buf);
            if((i%8 )== 0) sprintf(buf,"%s ",buf);
            i++;
        }
        sprintf(buf,"%s %s",buf,buf2);
    }

    if(tf_log.opened && tf_log.file != NULL) { 
        fprintf(tf_log.file, "%s\n", buf);
    } 

    log_stderr(buf);
}

