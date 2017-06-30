#ifndef _TF_LIST_H_
#define _TF_LIST_H_

/* Alias of `tclistval' but not checking size. */
#define TFLISTVAL(TF_ptr, TF_list, TF_index, TF_size) \
  do { \
    (TF_ptr) = (TF_list)->array[(TF_index)+(TF_list)->start].ptr; \
    (TF_size) = (TF_list)->array[(TF_index)+(TF_list)->start].size; \
  } while(false)

/* Alias of `tclistval' but not checking size and not using the third parameter. */
#define TFLISTVALPTR(TF_list, TF_index) \
  ((void *)((TF_list)->array[(TF_index)+(TF_list)->start].ptr))


/* Alias of `tclistval' but not checking size and returning the size of the value. */
#define TFLISTVALSIZ(TF_list, TF_index) \
  ((TF_list)->array[(TF_index)+(TF_list)->start].size)

typedef struct tf_list_node_st *tf_list_node_t;
typedef struct tf_list_st      *tf_list_t;

typedef struct tf_list_node_st{          /* type of structure for an element of a list */
  char *ptr;                             /* pointer to the region */
  int   size;                            /* size of the effective region */
} TF_LIST_NODE_ST;

typedef struct tf_list_st{               /* type of structure for an array list */
  tf_list_node_t array;                  /* array of data */
  int anum;                              /* number of the elements of the array */
  int start;                             /* start index of used elements */
  int num;                               /* number of used elements */
} TF_LIST_ST;

/* Create a list object.
   The return value is the new list object. */
tf_list_t tf_list_new(void);

/* Create a list object with expecting the number of elements.
   `anum' specifies the number of elements expected to be stored in the list.
   The return value is the new list object. */
tf_list_t tf_list_new2(int anum);

/* Create a list object with initial string elements.
   `str' specifies the string of the first element.
   The other arguments are other elements.  They should be trailed by a `NULL' argument.
   The return value is the new list object. */
tf_list_t tf_list_new3(const char *str, ...);

/* Copy a list object.
   `list' specifies the list object.
   The return value is the new list object equivalent to the specified object. */
tf_list_t tf_list_dup(const tf_list_t list);

/* Delete a list object.
   `list' specifies the list object.
   Note that the deleted object and its derivatives can not be used anymore. */
void tf_list_del(tf_list_t list);

/* Get the number of elements of a list object.
   `list' specifies the list object.
   The return value is the number of elements of the list. */
int tf_list_num(const tf_list_t list);

/* Get the pointer to the region of an element of a list object.
   `list' specifies the list object.
   `index' specifies the index of the element.
   `sp' specifies the pointer to the variable into which the size of the region of the return
   value is assigned.
   The return value is the pointer to the region of the value.
   Because an additional zero code is appended at the end of the region of the return value,
   the return value can be treated as a character string.  If `index' is equal to or more than
   the number of elements, the return value is `NULL'. */
const void *tf_list_val(const tf_list_t list, int index, int *sp);


/* Get the string of an element of a list object.
   `list' specifies the list object.
   `index' specifies the index of the element.
   The return value is the string of the value.
   If `index' is equal to or more than the number of elements, the return value is `NULL'. */
const char *tf_list_val2(const tf_list_t list, int index);

/* Add an element at the end of a list object.
   `list' specifies the list object.
   `ptr' specifies the pointer to the region of the new element.
   `size' specifies the size of the region. */
void tf_list_push(tf_list_t list, const void *ptr, int size);


/* Add a string element at the end of a list object.
   `list' specifies the list object.
   `str' specifies the string of the new element. */
void tf_list_push2(tf_list_t list, const char *str);


/* Remove an element of the end of a list object.
   `list' specifies the list object.
   `sp' specifies the pointer to the variable into which the size of the region of the return
   value is assigned.
   The return value is the pointer to the region of the removed element.
   Because an additional zero code is appended at the end of the region of the return value,
   the return value can be treated as a character string.  Because the region of the return
   value is allocated with the `malloc' call, it should be released with the `free' call when it
   is no longer in use.  If the list is empty, the return value is `NULL'. */
void *tf_list_pop(tf_list_t list, int *sp);


/* Remove a string element of the end of a list object.
   `list' specifies the list object.
   The return value is the string of the removed element.
   Because the region of the return value is allocated with the `malloc' call, it should be
   released with the `free' call when it is no longer in use.  If the list is empty, the return
   value is `NULL'. */
char *tf_list_pop2(tf_list_t list);

/* Add an element at the specified location of a list object.
   `list' specifies the list object.
   `index' specifies the index of the new element.
   `ptr' specifies the pointer to the region of the new element.
   `size' specifies the size of the region.
   If `index' is equal to or more than the number of elements, this function has no effect. */
void tf_list_insert(tf_list_t list, int index, const void *ptr, int size);


/* Add a string element at the specified location of a list object.
   `list' specifies the list object.
   `index' specifies the index of the new element.
   `str' specifies the string of the new element.
   If `index' is equal to or more than the number of elements, this function has no effect. */
void tf_list_insert2(tf_list_t list, int index, const char *str);


/* Remove an element at the specified location of a list object.
   `list' specifies the list object.
   `index' specifies the index of the element to be removed.
   `sp' specifies the pointer to the variable into which the size of the region of the return
   value is assigned.
   The return value is the pointer to the region of the removed element.
   Because an additional zero code is appended at the end of the region of the return value,
   the return value can be treated as a character string.  Because the region of the return
   value is allocated with the `malloc' call, it should be released with the `free' call when it
   is no longer in use.  If `index' is equal to or more than the number of elements, no element
   is removed and the return value is `NULL'. */
void *tf_list_remove(tf_list_t list, int index, int *sp);

/* Remove a string element at the specified location of a list object.
   `list' specifies the list object.
   `index' specifies the index of the element to be removed.
   The return value is the string of the removed element.
   Because the region of the return value is allocated with the `malloc' call, it should be
   released with the `free' call when it is no longer in use.  If `index' is equal to or more
   than the number of elements, no element is removed and the return value is `NULL'. */
char *tf_list_remove2(tf_list_t list, int index);


/* Overwrite an element at the specified location of a list object.
   `list' specifies the list object.
   `index' specifies the index of the element to be overwritten.
   `ptr' specifies the pointer to the region of the new content.
   `size' specifies the size of the new content.
   If `index' is equal to or more than the number of elements, this function has no effect. */
void tf_list_over(tf_list_t list, int index, const void *ptr, int size);


/* Overwrite a string element at the specified location of a list object.
   `list' specifies the list object.
   `index' specifies the index of the element to be overwritten.
   `str' specifies the string of the new content.
   If `index' is equal to or more than the number of elements, this function has no effect. */
void tf_list_over2(tf_list_t list, int index, const char *str);


/* Sort elements of a list object in lexical order.
   `list' specifies the list object. */
void tf_list_sort(tf_list_t list);

/* Search a list object for an element using liner search.
   `list' specifies the list object.
   `ptr' specifies the pointer to the region of the key.
   `size' specifies the size of the region.
   The return value is the index of a corresponding element or -1 if there is no corresponding
   element.
   If two or more elements correspond, the former returns. */
int tf_listl_search(const tf_list_t list, const void *ptr, int size);


/* Search a list object for an element using binary search.
   `list' specifies the list object.  It should be sorted in lexical order.
   `ptr' specifies the pointer to the region of the key.
   `size' specifies the size of the region.
   The return value is the index of a corresponding element or -1 if there is no corresponding
   element.
   If two or more elements correspond, which returns is not defined. */
int tf_list_bsearch(const tf_list_t list, const void *ptr, int size);


/* Clear a list object.
   `list' specifies the list object.
   All elements are removed. */
void tf_list_clear(tf_list_t list);

/* Serialize a list object into a byte array.
   `list' specifies the list object.
   `sp' specifies the pointer to the variable into which the size of the region of the return
   value is assigned.
   The return value is the pointer to the region of the result serial region.
   Because the region of the return value is allocated with the `malloc' call, it should be
   released with the `free' call when it is no longer in use. */
void *tf_list_dump(const tf_list_t list, int *sp);


/* Create a list object from a serialized byte array.
   `ptr' specifies the pointer to the region of serialized byte array.
   `size' specifies the size of the region.
   The return value is a new list object.
   Because the object of the return value is created with the function `tclistnew', it should
   be deleted with the function `tclistdel' when it is no longer in use. */
tf_list_t tf_list_load(const void *ptr, int size);

#endif /* _TF_LIST_H_ */
