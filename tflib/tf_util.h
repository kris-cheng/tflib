#ifndef _TF_UTIL_H_
#define _TF_UTIL_H_

#define TFNUMBUFSIZ    32                // size of a buffer for a number

/* set a buffer for a variable length number */
#define TFSETVNUMBUF(TF_len, TF_buf, TF_num) \
  do { \
    int _TF_num = (TF_num); \
    if(_TF_num == 0){ \
      ((signed char *)(TF_buf))[0] = 0; \
      (TF_len) = 1; \
    } else { \
      (TF_len) = 0; \
      while(_TF_num > 0){ \
        int _TF_rem = _TF_num & 0x7f; \
        _TF_num >>= 7; \
        if(_TF_num > 0){ \
          ((signed char *)(TF_buf))[(TF_len)] = -_TF_rem - 1; \
        } else { \
          ((signed char *)(TF_buf))[(TF_len)] = _TF_rem; \
        } \
        (TF_len)++; \
      } \
    } \
  } while(false)


/* read a variable length buffer */
#define TFREADVNUMBUF(TF_buf, TF_num, TF_step) \
  do { \
    TF_num = 0; \
    int _TF_base = 1; \
    int _TF_i = 0; \
    while(true){ \
      if(((signed char *)(TF_buf))[_TF_i] >= 0){ \
        TF_num += ((signed char *)(TF_buf))[_TF_i] * _TF_base; \
        break; \
      } \
      TF_num += _TF_base * (((signed char *)(TF_buf))[_TF_i] + 1) * -1; \
      _TF_base <<= 7; \
      _TF_i++; \
    } \
    (TF_step) = _TF_i + 1; \
  } while(false)

/* calculate the size of a buffer for a variable length number */
#define TFCALCVNUMSIZE(TF_num) \
  ((TF_num) < 0x80 ? 1 : (TF_num) < 0x4000 ? 2 : (TF_num) < 0x200000 ? 3 : \
   (TF_num) < 0x10000000 ? 4 : 5)

/* Compare two strings with case insensitive evaluation. */
int tf_stricmp(const char *astr, const char *bstr);

/* Cut space characters at head or tail of a string. */
char *tf_strtrim(char *str);

/* Check whether a string begins with a key.
   `str' specifies the target string.
   `key' specifies the forward matching key string.
   The return value is true if the target string begins with the key, else, it is false. */
bool tf_strfwm(const char *str, const char *key);

/* Check whether a string begins with a key with case insensitive evaluation.
   `str' specifies the target string.
   `key' specifies the forward matching key string.
   The return value is true if the target string begins with the key, else, it is false. */
bool tf_strifwm(const char *str, const char *key);


/* Check whether a string ends with a key.
   `str' specifies the target string.
   `key' specifies the backward matching key string.
   The return value is true if the target string ends with the key, else, it is false. */
bool tf_strbwm(const char *str, const char *key);

/* Check whether a string ends with a key with case insensitive evaluation.
   `str' specifies the target string.
   `key' specifies the backward matching key string.
   The return value is true if the target string ends with the key, else, it is false. */
bool tf_stribwm(const char *str, const char *key);

/* Create a list object by splitting a string.
   `str' specifies the source string.
   `delim' specifies a string containing delimiting characters.
   The return value is a list object of the split elements.
   If two delimiters are successive, it is assumed that an empty element is between the two.
   Because the object of the return value is created with the function `tclistnew', it should be
   deleted with the function `tclistdel' when it is no longer in use. */
tf_list_t tf_strsplit(const char *str, const char *delims);

/* Create a string by joining all elements of a list object.
   `list' specifies a list object.
   `delim' specifies a delimiting character.
   The return value is the result string.
   Because the region of the return value is allocated with the `malloc' call, it should be
   released with the `free' call when it is no longer in use. */
char *tf_strjoin(const tf_list_t list, char delim);


/* Check whether a string is numeric completely or not.
   `str' specifies the string to be checked.
   The return value is true if the string is numeric, else, it is false. */
bool tf_strisnum(const char *str);
int tf_num2strbin(uint64_t num, char *buf, int col, int fc);

/* Convert a string to an integer.
   `str' specifies the string.
   The return value is the integer.  If the string does not contain numeric expression, 0 is
   returned.
   This function is equivalent to `atoll' except that it does not depend on the locale. */
int64_t tf_atoi(const char *str);

/* Convert a hexadecimal string to an integer.
   `str' specifies the string.
   The return value is the integer.  If the string does not contain numeric expression, 0 is
   returned. */
int64_t tf_atoih(const char *str);

/* Convert a string with a metric prefix to an integer.
   `str' specifies the string, which can be trailed by a binary metric prefix.  "K", "M", "G",
   "T", "P", and "E" are supported.  They are case-insensitive.
   The return value is the integer.  If the string does not contain numeric expression, 0 is
   returned.  If the integer overflows the domain, `INT64_MAX' or `INT64_MIN' is returned
   according to the sign. */
int64_t tf_atoix(const char *str);

/* Convert a string to a real number.
   `str' specifies the string.
   The return value is the real number.  If the string does not contain numeric expression, 0.0
   is returned.
   This function is equivalent to `atof' except that it does not depend on the locale. */
double tf_atof(const char *str);

void tf_datetime_mmddhhmiss(char *str);
void tf_datetime(char *str);

/*
 * rate limiting
 */
typedef struct tf_rate_st *tf_rate_t;
typedef struct tf_rate_st
{
    __u32           total;      /* if we exceed this many events */
    __u32           count;      /* event count */
    __u32           seconds;    /* in this many seconds */
    __u32           wait;       /* then go bad for this many seconds */
    double          rate;

    time_t          time;       /* time we started counting events */
    time_t          bad;        /* time we went bad, or 0 if we're not */
}TF_RATE_ST;

tf_rate_t   tf_rate_new(__u32 total, __u32 seconds, __u32 wait);
void        tf_rate_free(tf_rate_t rt);
void        tf_rate_reset(tf_rate_t rt);

/**
  * Add a number of events to the counter.  This takes care of moving
  * the sliding window, if we've moved outside the previous window.
  */
void tf_rate_add(tf_rate_t rt, __u32 count);

/*
 * @return The amount of events we have left before we hit the rate
 * limit.  This could be number of bytes, or number of
 * connection attempts, etc.
 */
int tf_rate_left(tf_rate_t rt);

/**
 * @return 1 if we're under the rate limit and everything is fine or
 * 0 if the rate limit has been exceeded and we should throttle
 * something.
 */
bool tf_rate_check(tf_rate_t rt);


int asc_to_bin(char *hex,char *dsp,int count);
int bin_to_asc(char *dsp,char *hex,int count);

int asc_to_bcd(char *dst,char *src,int len);
int bcd_to_asc(char *dst,char *src,int len);

#endif
