#include "tflib.h"
#include "tf_conf_file.h"

static tf_keyval_t parse_line(char *str){
    char *key, *value, *ptr = tf_strtrim(str);
    int kk,ii = strlen(ptr);

    for (ii=0; (ptr[ii]=='\t')&&str[ii]!=0; ii++);

    if (ptr[0] == '#' || ptr[0] == 0) {
        return NULL; //comment line
    }

    tf_keyval_t kv = tf_alloc(sizeof(*kv));

    if (strncmp(ptr, "group", 5) == 0) {
        ptr += 5;
        key = ptr;
        for (ii=0; ptr[ii] != 0; ii++) {
            if (ptr[ii] == ' ' || ptr[ii] == '=' || ptr[ii] == '\t'){
                key++;
                continue;
            }
        }
        if (key[0] == 0) return NULL;
        kv->key   = tf_strnew2(key);
        kv->value = NULL;
        return kv;
    } else if (ptr[0] == '[') {
        ptr++;
        key = ptr;
        for (ii=0; ptr[ii] != 0 && ptr[ii] != ']'; ii++) {
            if (ptr[ii] == ' ' || ptr[ii] == '\t'){
                key++;
                continue;
            }
        }
        ptr[ii] = 0; //for ']'
        if (key[0] == 0) return NULL;
            
        kv->key   = tf_strnew2(key);
        kv->value = NULL;
        return kv;
    }

    bool found_key,found_value;
    found_key = found_value = false;
    key   = ptr;
    value = NULL;
    kk    = 0;
    for (ii=0 ; ptr[ii] != 0 ; ii++)
    {
        if (ptr[ii] == '#') { 
            ptr[ii] = 0;
            break;
        }

        if (ptr[ii] == ' ' || ptr[ii] == '\t' || ptr[ii] == '=' || ptr[ii] == ':'){
            if(!found_key){
                found_key = true;
                ptr[ii] = 0;
                kk=ii+1;
            }else{
                if(!found_value) {
                    kk = ii+1;
                }
            }
            continue;
        }

        if (found_key && !found_value) {
            found_value = true;
            value = &ptr[kk];
        }
    }

    if(key != NULL && value != NULL){
        kv->key   = tf_strnew2(key);
        kv->value = tf_strnew2(value);
        //debug(0,"k=%s value=%s", key, value);
        return kv;
    }
    tf_free(kv);
    return NULL;
}

cfg_group_t cfg_find_group(cfg_file_t cfg, char *name){

    cfg_group_t group = NULL;
    if(cfg==NULL || cfg->groups == NULL) return NULL;

    tf_list_node_t groups = cfg->groups->array;
    int end = cfg->groups->start + cfg->groups->num;
    for(int i = cfg->groups->start; i < end; i++){
        group = (cfg_group_t)groups[i].ptr;
        if(group!=NULL && tf_stricmp((char *)group->name, name)==0){
            return group;
        }
        group = NULL;
    }
    return group;
}

cfg_file_t cfg_create(char *fmt, ...) {
    va_list args;
    char filename[MAX_FILENAME_LEN*2];
    va_start(args, fmt);
    vsprintf(filename, fmt, args);
    va_end(args);

    tf_list_t file = tf_readlines(filename);
    if(file == NULL){
         error(errno, "Read config file:%s error", filename);
         return NULL;
    }

    cfg_file_t cfg = tf_alloc(sizeof(*cfg));
    cfg->groups = (tf_list_t)tf_list_new();

    cfg_group_t group = NULL;
    tf_keyval_t kv    = NULL;

    tf_list_node_t lines = file->array;
    int end = file->start + file->num;
    for(int i = file->start; i < end; i++){
        kv = parse_line(lines[i].ptr);
        if(kv == NULL ) continue;
        if(kv->value == NULL && kv->key != NULL){ //group name
            group = cfg_find_group(cfg, (char *)tf_strptr(kv->key));
            if(group == NULL){
                group = (cfg_group_t)tf_alloc(sizeof(*group));
                memset(group->name, 0, sizeof(group->name));
                memcpy(group->name, tf_strptr(kv->key), min(sizeof(group->name), tf_strsize(kv->key)));
                group->pairs = tf_list_new();
                tf_list_push(cfg->groups, group, sizeof(*group));
            }
            continue;
        } 

        if(group == NULL){
            debug(0, "Skip config line:%s", kv->key); 
            continue;
        }

        tf_list_push(group->pairs, kv, sizeof(*kv));
        tf_free(kv);
    }

    return cfg;
}

void cfg_destroy_pairs(tf_list_t pairs){
    if(pairs == NULL) return;

    tf_keyval_t kv = NULL;
    tf_list_node_t _nodes = pairs->array;
    int end = pairs->start + pairs->num;
    for(int i = pairs->start; i < end; i++){
        kv = (tf_keyval_t)_nodes[i].ptr;
        if(kv!=NULL){
            tf_strdel(kv->key);
            tf_strdel(kv->value);
        }
    }
}

void cfg_destroy(cfg_file_t cfg) {
    if(cfg == NULL) return;

    cfg_group_t group = NULL;
    tf_list_node_t groups = cfg->groups->array;
    int end = cfg->groups->start + cfg->groups->num;
    for(int i = cfg->groups->start; i < end; i++){
        group = (cfg_group_t)groups[i].ptr;
        if(group!=NULL){
            cfg_destroy_pairs(group->pairs);
            tf_list_del(group->pairs);
        }
    }
    tf_list_del(cfg->groups);
    tf_free((void *)cfg);
}

static tf_keyval_t cfg_get_kv(const cfg_group_t group, const char *key){
    if(group==NULL) return NULL;

    tf_keyval_t kv = NULL;
    tf_list_node_t _nodes = group->pairs->array;
    int end = group->pairs->start + group->pairs->num;
    for(int i = group->pairs->start; i < end; i++){
        kv = (tf_keyval_t)_nodes[i].ptr;
        if(kv!=NULL && memcmp(tf_strptr(kv->key), key, max(tf_strsize(kv->key),strlen(key))) == 0){
            return kv;
        }
    }
    return NULL;
}

const char *cfg_get_string(const cfg_group_t group, const char *key, const char *dft){
    tf_keyval_t kv = cfg_get_kv(group, key);
    if(kv!=NULL){
        return (const char *)tf_strptr(kv->value);
    }
    return dft;
}

bool cfg_get_bool(const cfg_group_t group, const char *key, bool dft) {
    tf_keyval_t kv = cfg_get_kv(group, key);
    if(kv!=NULL){
        return (tf_stricmp(tf_strptr(kv->value),"true")==0)||(tf_stricmp(tf_strptr(kv->value),"yes") == 0);
    }
    return dft;
}

int cfg_get_int(const cfg_group_t group, const char *key, int dft){
    tf_keyval_t kv = cfg_get_kv(group, key);
    if(kv!=NULL){
        return tf_atoi(tf_strptr(kv->value));
    }
    return dft;
}

int64_t cfg_get_intx(const cfg_group_t group, const char *key, int64_t dft){
    tf_keyval_t kv = cfg_get_kv(group, key);
    if(kv!=NULL){
        return tf_atoix(tf_strptr(kv->value));
    }
    return dft;
}

double cfg_get_float(const cfg_group_t group, const char *key, double dft){
    tf_keyval_t kv = cfg_get_kv(group, key);
    if(kv!=NULL){
        return tf_atof(tf_strptr(kv->value));
    }
    return dft;
}
