#ifndef _TF_MALLOC_H_
#define _TF_MALLOC_H_

#define __xstr(s) __str(s)
#define __str(s) #s

#if defined(TF_USE_TCMALLOC)
#define TF_MALLOC_LIB ("tcmalloc-" __xstr(TC_VERSION_MAJOR) "." __xstr(TC_VERSION_MINOR))
#include <google/tcmalloc.h>
#if (TC_VERSION_MAJOR == 1 && TC_VERSION_MINOR >= 6) || (TC_VERSION_MAJOR > 1)
#define HAVE_MALLOC_SIZE 1
#define tf_malloc_size(p) tc_malloc_size(p)
#else
#error "Newer version of tcmalloc required"
#endif
#endif /* TF_USE_TCMALLOC */

#ifndef TF_MALLOC_LIB
#define TF_MALLOC_LIB "libc"
#endif

void *tf_alloc(size_t size);
void *tf_calloc(size_t size);
void *tf_realloc(void *ptr, size_t size);
void *tf_memdup(const void *ptr, size_t size);
void  tf_free(void *ptr);

size_t tf_malloc_used_memory(void);
void tf_malloc_enable_thread_safeness(void);

#ifndef HAVE_MALLOC_SIZE
size_t tf_malloc_size(void *ptr);
#endif

#endif /* _TF_MALLOC_H_ */
