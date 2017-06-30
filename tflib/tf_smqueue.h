#ifndef _TF_SMQUEUE_H_
#define _TF_SMQUEUE_H_

/*
 * share memory circular queue definitions.
 */
#define SM_CIRCLEQ_HEAD(name)                    \
struct name {                                    \
    __u32 cqh_first;     /* first element */     \
    __u32 cqh_last;      /* last element */      \
    __u32 cqh_size;      /* size of element */      \
    void *cqh_ptr;       /* element point */      \
}

#define SM_CIRCLEQ_ENTRY                         \
struct {                                         \
    __u32 cqe_next;      /* next     element */  \
    __u32 cqe_prev;      /* previous element */  \
    __u32 cqe_index;     /* current  element */  \
}

/*
 * share memory circular queue functions.
 */
#define SM_CIRCLEQ_INIT(head, size, data) do {   \
    (head)->cqh_first = 0;                 \
    (head)->cqh_last  = 0;                 \
    (head)->cqh_size  = size;              \
    (head)->cqh_ptr   = data;              \
} while (/*CONSTCOND*/0)

#define SM_CIRCLEQ_ELM(head, index) ((head)->cqh_ptr+(head)->cqh_size * index)

#define SM_CIRCLEQ_INSERT_AFTER(head, listelm, elm, field) do {  \
    (elm)->field.cqe_next = (listelm)->field.cqe_next;           \
    (elm)->field.cqe_prev = (listelm)->field.cqe_index;          \
    if ((listelm)->field.cqe_next == (head)->cqh_first)          \
        (head)->cqh_last = (elm)->field.cqe_index;               \
    else                                \
        SM_CIRCLEQ_ELM(head, (listelm)->field.cqe_next)->field.cqe_prev = (elm)->cqe_index;  \
    (listelm)->field.cqe_next = (elm)->cqe_index;              \
} while (/*CONSTCOND*/0)

#define SM_CIRCLEQ_INSERT_BEFORE(head, listelm, elm, field) do {       \
    (elm)->field.cqe_next = (listelm);              \
    (elm)->field.cqe_prev = (listelm)->field.cqe_prev;      \
    if ((listelm)->field.cqe_prev == (void *)(head))        \
        (head)->cqh_first = (elm);              \
    else                                \
        (listelm)->field.cqe_prev->field.cqe_next = (elm);  \
    (listelm)->field.cqe_prev = (elm);              \
} while (/*CONSTCOND*/0)

/*
 * Circular queue access methods.
 */
#define SM_CIRCLEQ_EMPTY(head)     ((head)->cqh_first == 0 && (head)->cqh_ptr == NULL)
#define SM_CIRCLEQ_FIRST(head)     SM_CIRCLEQ_ELM(head, (head)->cqh_first)
#define SM_CIRCLEQ_LAST(head)      SM_CIRCLEQ_ELM(head, (head)->cqh_last)
#define SM_CIRCLEQ_NEXT(head, elm, field)    SM_CIRCLEQ_ELM(head, (elm)->field.cqe_next)
#define SM_CIRCLEQ_PREV(head, elm, field)    SM_CIRCLEQ_ELM(head, (elm)->field.cqe_prev)

#define SM_CIRCLEQ_LOOP_NEXT(head, elm, field)             \
    (((elm)->field.cqe_next == (head)->cqe_first)          \
        ? ((head)->cqh_first)                              \
        : (elm->field.cqe_next))
#define SM_CIRCLEQ_LOOP_PREV(head, elm, field)             \
    (((elm)->field.cqe_prev == (head)->cqe_first)          \
        ? ((head)->cqh_last)                               \
        : (elm->field.cqe_prev))

#endif
