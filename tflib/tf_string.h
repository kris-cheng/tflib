#ifndef _TF_STRING_H_
#define _TF_STRING_H_

#define TF_STR_UNIT     16                // allocation unit size of an extensible string

typedef struct tf_token_st  *tf_token_t;
typedef struct tf_token_st{
    __u32 length;
    __u8  *value;
}TF_TOKEN_ST;

typedef struct tf_str_st *tf_str_t;
typedef struct tf_str_st{
    __u32  size;                              /* size of the region */
    __u32  asize;                             /* size of the allocated region */
    __u8   *ptr;
}TF_STR_ST;

typedef struct tf_keyval_st *tf_keyval_t;
typedef struct tf_keyval_st{
    tf_str_t   key;
    tf_str_t   value;
}TF_KEYVAL_ST;

#define tf_string(str)         { sizeof(str) - 1, sizeof(str) , (__u8 *) str }
#define tf_null_string         { 0, 0, NULL }
#define tf_str_set(str, text)  (str)->size = sizeof(text) - 1; (str)->asize = sizeof(text); (str)->ptr = (__u8 *)text
#define tf_str_null(str)       (str)->size = 0; (str)->asize = 0; (str)->ptr = NULL

#define tf_tolower(c)          (__u8) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define tf_toupper(c)          (__u8) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)

tf_str_t tf_strnew(void);
tf_str_t tf_strnew2(const char *str);
tf_str_t tf_strnew3(int asiz);
tf_str_t tf_strdup(const tf_str_t xstr);
void tf_strdel(tf_str_t xstr);
void tf_strcat(tf_str_t xstr, const void *ptr, int size);
/* Concatenate a character string to the end of an extensible string object.*/
void tf_strcat2(tf_str_t xstr, const char *str);

/* Get the pointer of the region of an extensible string object.*/
const void *tf_strptr(const tf_str_t xstr);
int tf_strsize(const tf_str_t xstr);
/* Clear an extensible string object.*/
void tf_strclear(tf_str_t xstr);

/* Perform formatted output into an extensible string object.
   `xstr' specifies the extensible string object.
   `format' specifies the printf-like format string.  The conversion character `%' can be used
   with such flag characters as `s', `d', `o', `u', `x', `X', `c', `e', `E', `f', `g', `G', `@',
   `?', `b', and `%'.  `@' works as with `s' but escapes meta characters of XML.  `?' works as
   with `s' but escapes meta characters of URL.  `b' converts an integer to the string as binary
   numbers.  The other conversion character work as with each original.
   The other arguments are used according to the format string. */
void tf_strprintf(tf_str_t xstr, const char *format, ...);

 /* Allocate a formatted string on memory.
   `format' specifies the printf-like format string.  The conversion character `%' can be used
   with such flag characters as `s', `d', `o', `u', `x', `X', `c', `e', `E', `f', `g', `G', `@',
   `?', `b', and `%'.  `@' works as with `s' but escapes meta characters of XML.  `?' works as
   with `s' but escapes meta characters of URL.  `b' converts an integer to the string as binary
   numbers.  The other conversion character work as with each original.
   The other arguments are used according to the format string.
   The return value is the pointer to the region of the result string.
   Because the region of the return value is allocated with the `malloc' call, it should be
   released with the `free' call when it is no longer in use. */
char *tf_sprintf(const char *format, ...);

/* Convert an extensible string object into a usual allocated region.*/
void *tf_strtomalloc(tf_str_t xstr);
/* Create an extensible string object from an allocated region.*/
tf_str_t tf_strfrommalloc(void *ptr, int size);

#ifdef OS_SUNOS
char * strsep(char **s, const char *del);
#endif

#endif /* _TF_STRING_H_ */
