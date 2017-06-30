#ifndef _TF_CONF_FILE_H_
#define _TF_CONF_FILE_H_

typedef struct cfg_file_st  *cfg_file_t;
typedef struct cfg_group_st *cfg_group_t;

typedef struct cfg_group_st
{
    __u8       name[MAX_FILENAME_LEN];
    tf_list_t  pairs;
}CFG_GROUP_ST;

typedef struct cfg_file_st
{
    tf_list_t  groups;
}CFG_FILE_ST;

cfg_group_t cfg_find_group(cfg_file_t cfg, char *name);
cfg_file_t cfg_create(char *fmt, ...);
void cfg_destroy(cfg_file_t cfg);

const char *cfg_get_string(const cfg_group_t group, const char *key, const char *dft);
bool cfg_get_bool(const cfg_group_t group, const char *key, bool dft);
int  cfg_get_int(const cfg_group_t group, const char *key, int dft);
int64_t cfg_get_intx(const cfg_group_t group, const char *key, int64_t dft);
double  cfg_get_float(const cfg_group_t group, const char *key, double dft);

#endif
