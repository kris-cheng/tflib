#include "tflib.h"
#include "tf_thread.h"
#include <pthread.h>

static __u32   nthreads = 0;
static __u32   max_threads = TF_MAX_THREADS;
static bool    thread_inited = false;

static pthread_attr_t  thr_attr;

bool tf_create_thread(pthread_t *tid, tf_thread_value_t (*func)(void *arg), void *arg)
{
    int  err;

    if (nthreads >= max_threads) {
        crit(0, "no more than %u threads can be created", max_threads);
        return false;
    }

    if(thread_inited){
        err = pthread_create(tid, &thr_attr, func, arg);
    }else{
        err = pthread_create(tid, NULL, func, arg);
    }

    if (err != 0) {
        alert(errno, "pthread_create() failed");
        return false;
    }

    debug(0, "thread created: %u", *tid);
    nthreads++;

    return true;
}

bool tf_init_threads(int n, size_t size)
{
    int  err;
    max_threads = n;
    err = pthread_attr_init(&thr_attr);

    if (err != 0) {
        alert(errno, "pthread_attr_init() failed");
        return false;
    }

    err = pthread_attr_setstacksize(&thr_attr, size);

    if (err != 0) {
        alert(errno, "pthread_attr_setstacksize() failed");
        return false;
    }

    thread_inited = true;
    return true;
}


pthread_mutex_t *tf_mutex_init()
{
    int              err;
    pthread_mutex_t  *m;

    m = tf_alloc(sizeof(pthread_mutex_t));
    if (m == NULL) {
        return NULL;
    }

    err = pthread_mutex_init(m, NULL);
    if (err != 0) {
        alert(errno, "pthread_mutex_init() failed");
        return NULL;
    }

    return m;
}

void tf_mutex_destroy(pthread_mutex_t *m)
{
    int  err;
    err = pthread_mutex_destroy(m);
    if (err != 0) {
        alert(errno, "pthread_mutex_destroy(%p) failed", m);
    }
    tf_free(m);
}

void tf_mutex_lock(pthread_mutex_t *m)
{
    int  err;

    if (!thread_inited) {
        return;
    }

    debug(0, "lock mutex %p", m);

    err = pthread_mutex_lock(m);

    if (err != 0) {
        alert(errno, "pthread_mutex_lock(%p) failed", m);
        abort();
    }

    debug( 0, "mutex %p is locked", m);
}

int tf_mutex_trylock(pthread_mutex_t *m)
{
    int  err;

    if (!thread_inited) {
        return TF_OK;
    }

    debug( 0, "try lock mutex %p", m);

    err = pthread_mutex_trylock(m);
    if (err == EBUSY) {
        return TF_AGAIN;
    }
    if (err != 0) {
        alert(errno, "pthread_mutex_trylock(%p) failed", m);
        abort();
    }

    debug( 0, "mutex %p is locked", m);

    return TF_OK;
}

void tf_mutex_unlock(pthread_mutex_t *m)
{
    int  err;

    if (!thread_inited) {
        return;
    }

    debug( 0, "unlock mutex %p", m);

    err = pthread_mutex_unlock(m);

    if (err != 0) {
        alert(errno, "pthread_mutex_unlock(%p) failed", m);
        abort();
    }

    debug( 0, "mutex %p is unlocked", m);

    return;
}

pthread_cond_t *tf_cond_init()
{
    int             err;
    pthread_cond_t  *cv;

    cv = tf_alloc(sizeof(pthread_cond_t));
    if (cv == NULL) {
        return NULL;
    }

    err = pthread_cond_init(cv, NULL);
    if (err != 0) {
        alert(errno, "pthread_cond_init() failed");
        return NULL;
    }

    return cv;
}
    
void tf_cond_destroy(pthread_cond_t *cv)
{
    int  err;

    err = pthread_cond_destroy(cv);

    if (err != 0) {
        alert(errno, "pthread_cond_destroy(%p) failed", cv);
    }

    tf_free(cv);
}

bool tf_cond_wait(pthread_cond_t *cv, pthread_mutex_t *m)
{
    int  err;

    debug( 0, "cv %p wait", cv);

    err = pthread_cond_wait(cv, m);
    if (err != 0) {
        alert(errno, "pthread_cond_wait(%p) failed", cv);
        return false;
    }

    debug( 0, "cv %p is waked up", cv);
    debug( 0, "mutex %p is locked", m);

    return true;
}

bool tf_cond_signal(pthread_cond_t *cv)
{
    int  err;

    debug( 0, "cv %p to signal", cv);

    err = pthread_cond_signal(cv);
    if (err != 0) {
        alert(errno, "pthread_cond_signal(%p) failed", cv);
        return false;
    }

    debug( 0, "cv %p is signaled", cv);

    return true;
}

long tf_sleep(long microsec)
{
#ifndef OS_UNIXWARE
    struct timespec req;
    struct timespec rem;

    req.tv_sec = microsec/1000;
    req.tv_nsec = (microsec%1000) * 1000000;

    nanosleep(&req, &rem);

    return (rem.tv_sec * 1000) + (rem.tv_nsec / 1000000);
#else
    sleep(microsec/1000);
    return microsec;
#endif
}
