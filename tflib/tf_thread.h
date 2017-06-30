#ifndef _TF_THREAD_H_
#define _TF_THREAD_H_

#define TF_MAX_THREADS      128

pthread_mutex_t *tf_mutex_init();
void tf_mutex_destroy(pthread_mutex_t *m);

int  tf_mutex_trylock(pthread_mutex_t *m);
void tf_mutex_lock(pthread_mutex_t *m);
void tf_mutex_unlock(pthread_mutex_t *m);

pthread_cond_t *tf_cond_init();
void tf_cond_destroy(pthread_cond_t *cv);
bool tf_cond_wait(pthread_cond_t *cv, pthread_mutex_t *m);
bool tf_cond_signal(pthread_cond_t *cv);

typedef void *tf_thread_value_t;

bool tf_init_threads(int n, size_t size);
bool tf_create_thread(pthread_t *tid, tf_thread_value_t(*func)(void *arg), void *arg);

long tf_sleep(long microsec);

#endif
