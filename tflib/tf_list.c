#include "tflib.h"
#include "tf_list.h"

#define TF_LIST_UNIT     64                // allocation unit number of a list handle

/* private function prototypes */
static int tf_list_elemcmp(const void *a, const void *b);

/* Create a list object. */
tf_list_t tf_list_new(void){
    tf_list_t list;
    list = tf_alloc(sizeof(*list));
    list->anum = TF_LIST_UNIT;
    list->array = tf_alloc(sizeof(list->array[0]) * list->anum);
    list->start = 0;
    list->num = 0;
    return list;
}

/* Create a list object. */
tf_list_t tf_list_new2(int anum){
    tf_list_t list;
    list = tf_alloc(sizeof(*list));
    if(anum < 1) anum = 1;
    list->anum = anum;
    list->array = tf_alloc(sizeof(list->array[0]) * list->anum);
    list->start = 0;
    list->num = 0;
    return list;
}

/* Create a list object with initial string elements. */
tf_list_t tf_list_new3(const char *str, ...){
    tf_list_t list = tf_list_new();
    if(str){
        tf_list_push2(list, str);
        va_list ap;
        va_start(ap, str);
        const char *elem;
        while((elem = va_arg(ap, char *)) != NULL){
            tf_list_push2(list, elem);
        }
        va_end(ap);
    }
    return list;
}

/* Copy a list object. */
tf_list_t tf_list_dup(const tf_list_t list){
    assert(list);
    int num = list->num;
    if(num < 1) return tf_list_new();
    const tf_list_node_t array = (tf_list_node_t)( list->array + list->start );
    tf_list_t nlist;
    nlist = tf_alloc(sizeof(*nlist));
    tf_list_node_t narray = tf_alloc(sizeof(list->array[0]) * num);

    for(int i = 0; i < num; i++){
        int size = array[i].size;
        narray[i].ptr = tf_alloc(size + 1);
        memcpy(narray[i].ptr, array[i].ptr, size + 1);
        narray[i].size = array[i].size;
    }
    nlist->anum  = num;
    nlist->array = narray;
    nlist->start = 0;
    nlist->num = num;
    return nlist;
}

/* Delete a list object. */
void tf_list_del(tf_list_t list){
    assert(list);
    tf_list_node_t array = list->array;
    int end = list->start + list->num;
    for(int i = list->start; i < end; i++){
        tf_free(array[i].ptr);
    }
    tf_free(list->array);
    tf_free(list);
}

/* Get the number of elements of a list object. */
int tf_list_num(const tf_list_t list){
    assert(list);
    return list->num;
}

/* Get the pointer to the region of an element of a list object. */
const void *tf_list_val(const tf_list_t list, int index, int *sp){
    assert(list && index >= 0 && sp);
    if(index >= list->num) return NULL;
    index += list->start;
    *sp = list->array[index].size;
    return list->array[index].ptr;
}

/* Get the string of an element of a list object. */
const char *tf_list_val2(const tf_list_t list, int index){
    assert(list && index >= 0);
    if(index >= list->num) return NULL;
    index += list->start;
    return list->array[index].ptr;
}

/* Add an element at the end of a list object. */
void tf_list_push(tf_list_t list, const void *ptr, int size){
    assert(list && ptr && size >= 0);
    int index = list->start + list->num;
    if(index >= list->anum){
        list->anum += list->num + 1;
        list->array = tf_realloc( list->array, list->anum * sizeof(list->array[0]));
    }
    tf_list_node_t array = list->array;
    array[index].ptr = tf_alloc(size + 1);
    memcpy(array[index].ptr, ptr, size);
    array[index].ptr[size] = '\0';
    array[index].size = size;
    list->num++;
}

/* Add a string element at the end of a list object. */
void tf_list_push2(tf_list_t list, const char *str){
    assert(list && str);
    int index = list->start + list->num;
    if(index >= list->anum){
        list->anum += list->num + 1;
        list->array = tf_realloc( list->array, list->anum * sizeof(list->array[0]));
    }
    int size = strlen(str);
    tf_list_node_t array = list->array;
    array[index].ptr = tf_alloc(size + 1);
    memcpy(array[index].ptr, str, size + 1);
    array[index].size = size;
    list->num++;
}

/* Remove an element of the end of a list object. */
void *tf_list_pop(tf_list_t list, int *sp){
    assert(list && sp);
    if(list->num < 1) return NULL;
    int index = list->start + list->num - 1;
    list->num--;
    *sp = list->array[index].size;
    return list->array[index].ptr;
}

/* Remove a string element of the end of a list object. */
char *tf_list_pop2(tf_list_t list){
    assert(list);
    if(list->num < 1) return NULL;
    int index = list->start + list->num - 1;
    list->num--;
    return list->array[index].ptr;
}

/* Add an element at the specified location of a list object. */
void tf_list_insert(tf_list_t list, int index, const void *ptr, int size){
    assert(list && index >= 0 && ptr && size >= 0);
    if(index > list->num) return;
    index += list->start;
    if(list->start + list->num >= list->anum){
        list->anum += list->num + 1;
        list->array = tf_realloc( list->array, list->anum * sizeof(list->array[0]));
    }

    memmove(list->array + index + 1, list->array + index,
            sizeof(list->array[0]) * (list->start + list->num - index));

    list->array[index].ptr = tf_alloc(size + 1);
    memcpy(list->array[index].ptr, ptr, size);
    list->array[index].ptr[size] = '\0';
    list->array[index].size = size;
    list->num++;
}

/* Add a string element at the specified location of a list object. */
void tf_list_insert2(tf_list_t list, int index, const char *str){
    assert(list && index >= 0 && str);
    if(index > list->num) return;
    index += list->start;
    if(list->start + list->num >= list->anum){
        list->anum += list->num + 1;
        list->array = tf_realloc( list->array, list->anum * sizeof(list->array[0]));
    }
    memmove(list->array + index + 1, list->array + index,
            sizeof(list->array[0]) * (list->start + list->num - index));
    int size = strlen(str);
    list->array[index].ptr = tf_alloc(size + 1);
    memcpy(list->array[index].ptr, str, size);
    list->array[index].ptr[size] = '\0';
    list->array[index].size = size;
    list->num++;
}


/* Remove an element at the specified location of a list object. */
void *tf_list_remove(tf_list_t list, int index, int *sp){
    assert(list && index >= 0 && sp);
    if(index >= list->num) return NULL;
    index += list->start;
    void *rv = list->array[index].ptr;
    *sp = list->array[index].size;
    list->num--;
    memmove(list->array + index, list->array + index + 1,
            sizeof(list->array[0]) * (list->start + list->num - index));
    return rv;
}
   
/* Remove a string element at the specified location of a list object. */
char *tf_list_remove2(tf_list_t list, int index){
    assert(list && index >= 0);
    if(index >= list->num) return NULL;
    index += list->start;
    void *rv = list->array[index].ptr;
    list->num--;
    memmove(list->array + index, list->array + index + 1,
            sizeof(list->array[0]) * (list->start + list->num - index));
    return rv;
}


/* Overwrite an element at the specified location of a list object. */
void tf_list_over(tf_list_t list, int index, const void *ptr, int size){
    assert(list && index >= 0 && ptr && size >= 0);
    if(index >= list->num) return;
    index += list->start;
    if(size > list->array[index].size)
        list->array[index].ptr = tf_realloc( list->array[index].ptr, size + 1);
    memcpy(list->array[index].ptr, ptr, size);
    list->array[index].size = size;
    list->array[index].ptr[size] = '\0';
}

/* Overwrite a string element at the specified location of a list object. */
void tf_list_over2(tf_list_t list, int index, const char *str){
    assert(list && index >= 0 && str);
    if(index >= list->num) return;
    index += list->start;
    int size = strlen(str);
    if(size > list->array[index].size)
        list->array[index].ptr = tf_realloc( list->array[index].ptr, size + 1);
    memcpy(list->array[index].ptr, str, size + 1);
    list->array[index].size = size;
}


/* Sort elements of a list object in lexical order. */
void tf_list_sort(tf_list_t list){
    assert(list);
    qsort(list->array + list->start, list->num, sizeof(list->array[0]), tf_list_elemcmp);
}

/* Search a list object for an element using liner search. */
int tf_listl_search(const tf_list_t list, const void *ptr, int size){
    assert(list && ptr && size >= 0);
    int end = list->start + list->num;
    for(int i = list->start; i < end; i++){
        if(list->array[i].size == size && !memcmp(list->array[i].ptr, ptr, size))
        return i - list->start;
    }
    return -1;
}


/* Search a list object for an element using binary search. */
int tf_list_bsearch(const tf_list_t list, const void *ptr, int size){
    assert(list && ptr && size >= 0);
    TF_LIST_NODE_ST key;
    key.ptr = (char *)ptr;
    key.size = size;
    tf_list_node_t res = bsearch(&key, list->array + list->start,
                             list->num, sizeof(list->array[0]), tf_list_elemcmp);

    return res ? res - list->array - list->start : -1;
}


/* Clear a list object. */
void tf_list_clear(tf_list_t list){
    assert(list);
    tf_list_node_t array = list->array;
    int end = list->start + list->num;
    for(int i = list->start; i < end; i++){
        tf_free(array[i].ptr);
    }
    list->start = 0;
    list->num = 0;
}

/* Serialize a list object into a byte array. */
void *tf_list_dump(const tf_list_t list, int *sp){
    assert(list && sp);
    const tf_list_node_t array = list->array;
    int end = list->start + list->num;
    int tsiz = 0;
    for(int i = list->start; i < end; i++){
        tsiz += array[i].size + sizeof(int);
    }
    char *buf = tf_alloc( tsiz + 1);
    char *wp = buf;
    for(int i = list->start; i < end; i++){
        int step;
        TFSETVNUMBUF(step, wp, array[i].size);
        wp += step;
        memcpy(wp, array[i].ptr, array[i].size);
        wp += array[i].size;
    }
    *sp = wp - buf;
    return buf;
}

/* Create a list object from a serialized byte array. */
tf_list_t tf_list_load(const void *ptr, int size){
    assert(ptr && size >= 0);
    tf_list_t list;
    list = tf_alloc(sizeof(*list));
    int anum = size / sizeof(int) + 1;
    tf_list_node_t array;
    array = tf_alloc( sizeof(array[0]) * anum);
    int num = 0;
    const char *rp = ptr;
    const char *ep = (char *)ptr + size;

    while(rp < ep){
        int step, vsiz;
        TFREADVNUMBUF(rp, vsiz, step);
        rp += step;
        if(num >= anum){
            anum *= 2;
            array = tf_realloc( array, anum * sizeof(array[0]));
        }

        array[num].ptr = tf_alloc(vsiz + 1);
        memcpy(array[num].ptr, rp, vsiz);
        array[num].ptr[vsiz] = '\0';
        array[num].size = vsiz;
        num++;
        rp += vsiz;
    }
    list->anum = anum;
    list->array = array;
    list->start = 0;
    list->num = num;
    return list;
}

/* Compare two list elements in lexical order.
   `a' specifies the pointer to one element.
   `b' specifies the pointer to the other element.
   The return value is positive if the former is big, negative if the latter is big, 0 if both
   are equivalent. */
static int tf_list_elemcmp(const void *a, const void *b){
    assert(a && b);
    unsigned char *ao = (unsigned char *)((tf_list_node_t)a)->ptr;
    unsigned char *bo = (unsigned char *)((tf_list_node_t)b)->ptr;
    int size = (((tf_list_node_t)a)->size < ((tf_list_node_t)b)->size) ?
        ((tf_list_node_t)a)->size : ((tf_list_node_t)b)->size;
    for(int i = 0; i < size; i++){
        if(ao[i] > bo[i]) return 1;
        if(ao[i] < bo[i]) return -1;
    }
    return ((tf_list_node_t)a)->size - ((tf_list_node_t)b)->size;
}
