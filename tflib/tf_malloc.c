#include "tflib.h"
#include "tf_malloc.h"
#include <pthread.h>

#ifdef HAVE_MALLOC_SIZE
#define PREFIX_SIZE (0)
#else
#if defined(__sun) || defined(__sparc) || defined(__sparc__)
#define PREFIX_SIZE (sizeof(long long))
#else
#define PREFIX_SIZE (sizeof(size_t))
#endif
#endif

/* Explicitly override malloc/free etc when using tcmalloc. */
#if defined(TF_USE_TCMALLOC)
#define malloc(size) tc_malloc(size)
#define calloc(count,size) tc_calloc(count,size)
#define realloc(ptr,size)  tc_realloc(ptr,size)
#define free(ptr) tc_free(ptr)
#endif

#ifdef HAVE_ATOMIC
#define tf_malloc_stat_add(__n) __sync_add_and_fetch(&used_memory, (__n))
#define tf_malloc_stat_sub(__n) __sync_sub_and_fetch(&used_memory, (__n))
#else
#define tf_malloc_stat_add(__n) do { \
    pthread_mutex_lock(&used_memory_mutex); \
    used_memory += (__n); \
    pthread_mutex_unlock(&used_memory_mutex); \
} while(0)

#define tf_malloc_stat_sub(__n) do { \
    pthread_mutex_lock(&used_memory_mutex); \
    used_memory -= (__n); \
    pthread_mutex_unlock(&used_memory_mutex); \
} while(0)

#endif

#define tf_malloc_stat_alloc(__n) do { \
    size_t _n = (__n); \
    if (_n&(sizeof(long)-1)) _n += sizeof(long)-(_n&(sizeof(long)-1)); \
    if (tf_malloc_thread_safe) { \
        tf_malloc_stat_add(_n); \
    } else { \
        used_memory += _n; \
    } \
} while(0)

#define tf_malloc_stat_free(__n) do { \
    size_t _n = (__n); \
    if (_n&(sizeof(long)-1)) _n += sizeof(long)-(_n&(sizeof(long)-1)); \
    if (tf_malloc_thread_safe) { \
        tf_malloc_stat_sub(_n); \
    } else { \
        used_memory -= _n; \
    } \
} while(0)

static size_t used_memory = 0;
static bool tf_malloc_thread_safe = false;
pthread_mutex_t used_memory_mutex = PTHREAD_MUTEX_INITIALIZER;


static void tf_malloc_default_oom(size_t size) {
    emerg(errno, "tf_malloc: Out of memory trying to allocate %zu bytes", size);
    abort();
}

static void (*tf_malloc_oom_handler)(size_t) = tf_malloc_default_oom;

void *tf_alloc(size_t size) {
    void *ptr = malloc(size+PREFIX_SIZE);

    if (!ptr) tf_malloc_oom_handler(size);
#ifdef HAVE_MALLOC_SIZE
    tf_malloc_stat_alloc(tf_malloc_size(ptr));
    return ptr;
#else
    *((size_t*)ptr) = size;
    tf_malloc_stat_alloc(size+PREFIX_SIZE);
    return (char*)ptr+PREFIX_SIZE;
#endif
}

void *tf_calloc(size_t size) {
    void *ptr = calloc(1, size+PREFIX_SIZE);

    if (!ptr) tf_malloc_oom_handler(size);
#ifdef HAVE_MALLOC_SIZE
    tf_malloc_stat_alloc(tf_malloc_size(ptr));
    return ptr;
#else
    *((size_t*)ptr) = size;
    tf_malloc_stat_alloc(size+PREFIX_SIZE);
    return (char*)ptr+PREFIX_SIZE;
#endif
}

#ifndef HAVE_MALLOC_SIZE
size_t tf_malloc_size(void *ptr) {
    void *realptr = (char*)ptr-PREFIX_SIZE;
    size_t size = *((size_t*)realptr);
    /* Assume at least that all the allocations are padded at sizeof(long) by
 *      * the underlying allocator. */
    if (size&(sizeof(long)-1)) size += sizeof(long)-(size&(sizeof(long)-1));
    return size+PREFIX_SIZE;
}
#endif

void *tf_realloc(void *ptr, size_t size) {
#ifndef HAVE_MALLOC_SIZE
    void *realptr;
#endif
    size_t oldsize;
    void *newptr;

    if (ptr == NULL) return tf_alloc(size);
#ifdef HAVE_MALLOC_SIZE
    oldsize = tf_malloc_size(ptr);
    newptr = realloc(ptr,size);
    if (!newptr) tf_malloc_oom_handler(size);

    tf_malloc_stat_free(oldsize);
    tf_malloc_stat_alloc(zmalloc_size(newptr));
    return newptr;
#else
    realptr = (char*)ptr-PREFIX_SIZE;
    oldsize = *((size_t*)realptr);
    newptr = realloc(realptr,size+PREFIX_SIZE);
    if (!newptr) tf_malloc_oom_handler(size);

    *((size_t*)newptr) = size;
    tf_malloc_stat_free(oldsize);
    tf_malloc_stat_alloc(size);
    return (char*)newptr+PREFIX_SIZE;
#endif
}

void tf_free(void *ptr) {
#ifndef HAVE_MALLOC_SIZE
    void *realptr;
    size_t oldsize;
#endif

    if (ptr == NULL) return;
#ifdef HAVE_MALLOC_SIZE
    tf_malloc_stat_free(tf_malloc_size(ptr));
    free(ptr);
#else
    realptr = (char*)ptr-PREFIX_SIZE;
    oldsize = *((size_t*)realptr);
    tf_malloc_stat_free(oldsize+PREFIX_SIZE);
    free(realptr);
#endif
}

size_t tf_malloc_used_memory(void) {
    size_t um;

    if (tf_malloc_thread_safe) {
#ifdef HAVE_ATOMIC
        um = __sync_add_and_fetch(&used_memory, 0);
#else
        pthread_mutex_lock(&used_memory_mutex);
        um = used_memory;
        pthread_mutex_unlock(&used_memory_mutex);
#endif
    }
    else {
        um = used_memory;
    }
    return um;
}

void tf_malloc_enable_thread_safeness(void) {
    tf_malloc_thread_safe = true;
}

/* Duplicate a region on memory. */
void *tf_memdup(const void *ptr, size_t size){
    char *p = tf_alloc(size + 1);
    memcpy(p, ptr, size);
    p[size] = '\0';
    return p;
}

