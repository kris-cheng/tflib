#include "tflib.h"
#include "tf_string.h"

/* private function prototypes */
static void tf_vstrprintf(tf_str_t xstr, const char *format, va_list ap);

/* Create an extensible string object. */
tf_str_t tf_strnew(void){
    tf_str_t xstr;
    xstr = tf_alloc( sizeof(*xstr));
    xstr->ptr   = tf_alloc(TF_STR_UNIT);
    xstr->size  = 0;
    xstr->asize = TF_STR_UNIT;
    xstr->ptr[0] = '\0';
    return xstr;
}

/* Create an extensible string object from a character string. */
tf_str_t tf_strnew2(const char *str){
    assert(str);
    tf_str_t xstr = tf_alloc(sizeof(*xstr));
    int size = strlen(str);
    int asize = max(size + 1, TF_STR_UNIT);
    xstr->ptr = tf_alloc(asize);
    xstr->size = size;
    xstr->asize = asize;
    memcpy(xstr->ptr, str, size + 1);
    return xstr;
}

/* Create an extensible string object with the initial allocation size. */
tf_str_t tf_strnew3(int asiz){
    assert(asiz >= 0);
    asiz = max(asiz, TF_STR_UNIT);
    tf_str_t xstr = tf_alloc(sizeof(*xstr));
    xstr->ptr = tf_alloc(asiz);
    xstr->size = 0;
    xstr->asize = asiz;
    xstr->ptr[0] = '\0';
    return xstr;
}

/* Copy an extensible string object. */
tf_str_t tf_strdup(const tf_str_t xstr){
    assert(xstr);
    tf_str_t nxstr = tf_alloc(sizeof(*nxstr));
    int asize = max(xstr->size + 1, TF_STR_UNIT);
    nxstr->ptr = tf_alloc(asize);
    nxstr->size = xstr->size;
    nxstr->asize = asize;
    memcpy(nxstr->ptr, xstr->ptr, xstr->size + 1);
    return nxstr;
}

/* Delete an extensible string object. */
void tf_strdel(tf_str_t xstr){
    assert(xstr);
    tf_free(xstr->ptr);
    tf_free(xstr);
}

/* Concatenate a region to the end of an extensible string object. */
void tf_strcat(tf_str_t xstr, const void *ptr, int size){
    assert(xstr && ptr && size >= 0);
    int nsize = xstr->size + size + 1;
    if(xstr->asize < nsize){
        while(xstr->asize < nsize){
            xstr->asize *= 2;
            if(xstr->asize < nsize) xstr->asize = nsize;
        }
        xstr->ptr = tf_realloc(xstr->ptr, xstr->asize);
    }
    memcpy(xstr->ptr + xstr->size, ptr, size);
    xstr->size += size;
    xstr->ptr[xstr->size] = '\0';
}

/* Concatenate a character string to the end of an extensible string object. */
void tf_strcat2(tf_str_t xstr, const char *str){
    assert(xstr && str);
    int size = strlen(str);
    int nsize = xstr->size + size + 1;
    if(xstr->asize < nsize){
        while(xstr->asize < nsize){
            xstr->asize *= 2;
            if(xstr->asize < nsize) xstr->asize = nsize;
        }
        xstr->ptr = tf_realloc(xstr->ptr, xstr->asize);
    }
    memcpy(xstr->ptr + xstr->size, str, size + 1);
    xstr->size += size;
}

/* Get the pointer of the region of an extensible string object. */
const void *tf_strptr(const tf_str_t xstr){
    assert(xstr);
    return xstr->ptr;
}

/* Get the size of the region of an extensible string object. */
int tf_strsize(const tf_str_t xstr){
    assert(xstr);
    return xstr->size;
}

/* Clear an extensible string object. */
void tf_strclear(tf_str_t xstr){
    assert(xstr);
    xstr->size = 0;
    xstr->ptr[0] = '\0';
}

/* Perform formatted output into an extensible string object. */
void tf_strprintf(tf_str_t xstr, const char *format, ...){
    assert(xstr && format);
    va_list ap;
    va_start(ap, format);
    tf_vstrprintf(xstr, format, ap);
    va_end(ap);
}


/* Allocate a formatted string on memory. */
char *tf_sprintf(const char *format, ...){
    assert(format);
    tf_str_t xstr = tf_strnew();
    va_list ap;
    va_start(ap, format);
    tf_vstrprintf(xstr, format, ap);
    va_end(ap);
    return tf_strtomalloc(xstr);
}

/* Perform formatted output into an extensible string object. */
static void tf_vstrprintf(tf_str_t xstr, const char *format, va_list ap){
  assert(xstr && format);
  while(*format != '\0'){
    if(*format == '%'){
      char cbuf[TFNUMBUFSIZ];
      cbuf[0] = '%';
      int cblen = 1;
      int lnum = 0;
      format++;
      while(strchr("0123456789 .+-hlLz", *format) && *format != '\0' &&
            cblen < TFNUMBUFSIZ - 1){
        if(*format == 'l' || *format == 'L') lnum++;
        cbuf[cblen++] = *(format++);
      }
      cbuf[cblen++] = *format;
      cbuf[cblen] = '\0';
      int tlen;
      char *tmp, tbuf[TFNUMBUFSIZ*4];
      switch(*format){
        case 's':
          tmp = va_arg(ap, char *);
          if(!tmp) tmp = "(null)";
          tf_strcat2(xstr, tmp);
          break;
        case 'd':
          if(lnum >= 2){
            tlen = sprintf(tbuf, cbuf, va_arg(ap, long long));
          } else if(lnum >= 1){
            tlen = sprintf(tbuf, cbuf, va_arg(ap, long));
          } else {
            tlen = sprintf(tbuf, cbuf, va_arg(ap, int));
          }
          tf_strcat(xstr, tbuf, tlen);
          break;
        case 'o': case 'u': case 'x': case 'X': case 'c':
          if(lnum >= 2){
            tlen = sprintf(tbuf, cbuf, va_arg(ap, unsigned long long));
          } else if(lnum >= 1){
            tlen = sprintf(tbuf, cbuf, va_arg(ap, unsigned long));
          } else {
            tlen = sprintf(tbuf, cbuf, va_arg(ap, unsigned int));
          }
          tf_strcat(xstr, tbuf, tlen);
          break;
        case 'e': case 'E': case 'f': case 'g': case 'G':
          if(lnum >= 1){
            tlen = snprintf(tbuf, sizeof(tbuf), cbuf, va_arg(ap, long double));
          } else {
            tlen = snprintf(tbuf, sizeof(tbuf), cbuf, va_arg(ap, double));
          }
          if(tlen < 0 || tlen > sizeof(tbuf)){
            tbuf[sizeof(tbuf)-1] = '*';
            tlen = sizeof(tbuf);
          }
          tf_strcat(xstr, tbuf, tlen);
          break;
        case '@':
          tmp = va_arg(ap, char *);
          if(!tmp) tmp = "(null)";
          while(*tmp){
            switch(*tmp){
              case '&': tf_strcat(xstr, "&amp;", 5); break;
              case '<': tf_strcat(xstr, "&lt;", 4); break;
              case '>': tf_strcat(xstr, "&gt;", 4); break;
              case '"': tf_strcat(xstr, "&quot;", 6); break;
              default:
                if(!((*tmp >= 0 && *tmp <= 0x8) || (*tmp >= 0x0e && *tmp <= 0x1f)))
                  tf_strcat(xstr, tmp, 1);
                break;
            }
            tmp++;
          }
          break;
        case '?':
          tmp = va_arg(ap, char *);
          if(!tmp) tmp = "(null)";
          while(*tmp){
            unsigned char c = *(unsigned char *)tmp;
            if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
               (c >= '0' && c <= '9') || (c != '\0' && strchr("_-.", c))){
              tf_strcat(xstr, tmp, 1);
            } else {
              tlen = sprintf(tbuf, "%%%02X", c);
              tf_strcat(xstr, tbuf, tlen);
            }
            tmp++;
          }
          break;
        case 'b':
          if(lnum >= 2){
            tlen = tf_num2strbin(va_arg(ap, unsigned long long), tbuf,
                                 tf_atoi(cbuf + 1), (cbuf[1] == '0') ? '0' : ' ');
          } else if(lnum >= 1){
            tlen = tf_num2strbin(va_arg(ap, unsigned long), tbuf,
                                 tf_atoi(cbuf + 1), (cbuf[1] == '0') ? '0' : ' ');
          } else {
            tlen = tf_num2strbin(va_arg(ap, unsigned int), tbuf,
                                 tf_atoi(cbuf + 1), (cbuf[1] == '0') ? '0' : ' ');
          }
          tf_strcat(xstr, tbuf, tlen);
          break;
        case '%':
          tf_strcat(xstr, "%", 1);
          break;
      }
    } else {
      tf_strcat(xstr, format, 1);
    }
    format++;
  }
}

/* Convert an extensible string object into a usual allocated region. */
void *tf_strtomalloc(tf_str_t xstr){
    assert(xstr);
    __u8 *ptr = xstr->ptr;
    tf_free(xstr);
    return ptr;
}


/* Create an extensible string object from an allocated region. */
tf_str_t tf_strfrommalloc(void *ptr, int size){
  tf_str_t xstr = tf_alloc(sizeof(*xstr));
  xstr->ptr = tf_realloc( ptr, size + 1);
  xstr->ptr[size] = '\0';
  xstr->size  = size;
  xstr->asize = size;
  return xstr;
}

#ifdef OS_SUNOS

char * strsep(char **s, const char *del)
{
    char *d, *tok;
    if (!s || !*s)
        return NULL;
    tok = *s;
    d = strstr(tok, del);
    if (d) {
        *d = '\0';
        *s = d + 1;
    } else
        *s = NULL;
    return tok;
}

#endif
