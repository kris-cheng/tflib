#include "tflib.h"
#include "tf_util.h"
#include <math.h>
#include <ctype.h>

#define TFRANDDEV      "/dev/urandom"    // path of the random device file
#define TFDISTMAXLEN   4096              // maximum size of a string for distance checking
#define TFDISTBUFSIZ   16384             // size of a distance buffer
#define TFLDBLCOLMAX   16                // maximum number of columns of the long double

/* Check whether a string is numeric completely or not. */
bool tf_strisnum(const char *str){
  assert(str);
  bool isnum = false;
  while(*str > '\0' && *str <= ' '){
    str++;
  }
  if(*str == '-') str++;
  while(*str >= '0' && *str <= '9'){
    isnum = true;
    str++;
  }
  if(*str == '.') str++;
  while(*str >= '0' && *str <= '9'){
    isnum = true;
    str++;
  }
  while(*str > '\0' && *str <= ' '){
    str++;
  }
  return isnum && *str == '\0';
}

/* Convert a hexadecimal string to an integer. */
int64_t tf_atoih(const char *str){
  assert(str);
  while(*str > '\0' && *str <= ' '){
    str++;
  }
  if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X')){
    str += 2;
  }
  int64_t num = 0;
  while(true){
    if(*str >= '0' && *str <= '9'){
      num = num * 0x10 + *str - '0';
    } else if(*str >= 'a' && *str <= 'f'){
      num = num * 0x10 + *str - 'a' + 10;
    } else if(*str >= 'A' && *str <= 'F'){
      num = num * 0x10 + *str - 'A' + 10;
    } else {
      break;
    }
    str++;
  }
  return num;
}

/* Compare two strings with case insensitive evaluation. */
int tf_stricmp(const char *astr, const char *bstr){
  assert(astr && bstr);
  while(*astr != '\0'){
    if(*bstr == '\0') return 1;
    int ac = (*astr >= 'A' && *astr <= 'Z') ? *astr + ('a' - 'A') : *(unsigned char *)astr;
    int bc = (*bstr >= 'A' && *bstr <= 'Z') ? *bstr + ('a' - 'A') : *(unsigned char *)bstr;
    if(ac != bc) return ac - bc;
    astr++;
    bstr++;
  }
  return (*bstr == '\0') ? 0 : -1;
}

/* Cut space characters at head or tail of a string. */
char *tf_strtrim(char *str){
  assert(str);
  const char *rp = str;
  char *wp = str;
  bool head = true;
  while(*rp != '\0'){
    if(*rp > '\0' && *rp <= ' '){
      if(!head) *(wp++) = *rp;
    } else {
      *(wp++) = *rp;
      head = false;
    }
    rp++;
  }
  *wp = '\0';
  while(wp > str && wp[-1] > '\0' && wp[-1] <= ' '){
    *(--wp) = '\0';
  }
  return str;
}

/* Check whether a string begins with a key. */
bool tf_strfwm(const char *str, const char *key){
  assert(str && key);
  while(*key != '\0'){
    if(*str != *key || *str == '\0') return false;
    key++;
    str++;
  }
  return true;
}

/* Check whether a string begins with a key with case insensitive evaluation. */
bool tf_strifwm(const char *str, const char *key){
  assert(str && key);
  while(*key != '\0'){
    if(*str == '\0') return false;
    int sc = *str;
    if(sc >= 'A' && sc <= 'Z') sc += 'a' - 'A';
    int kc = *key;
    if(kc >= 'A' && kc <= 'Z') kc += 'a' - 'A';
    if(sc != kc) return false;
    key++;
    str++;
  }
  return true;
}


/* Check whether a string ends with a key. */
bool tf_strbwm(const char *str, const char *key){
  assert(str && key);
  int slen = strlen(str);
  int klen = strlen(key);
  for(int i = 1; i <= klen; i++){
    if(i > slen || str[slen-i] != key[klen-i]) return false;
  }
  return true;
}

/* Check whether a string ends with a key with case insensitive evaluation. */
bool tf_stribwm(const char *str, const char *key){
  assert(str && key);
  int slen = strlen(str);
  int klen = strlen(key);
  for(int i = 1; i <= klen; i++){
    if(i > slen) return false;
    int sc = str[slen-i];
    if(sc >= 'A' && sc <= 'Z') sc += 'a' - 'A';
    int kc = key[klen-i];
    if(kc >= 'A' && kc <= 'Z') kc += 'a' - 'A';
    if(sc != kc) return false;
  }
  return true;
}

/* Create a list object by splitting a string. */
tf_list_t tf_strsplit(const char *str, const char *delims){
  assert(str && delims);
  tf_list_t list = tf_list_new();
  while(true){
    const char *sp = str;
    while(*str != '\0' && !strchr(delims, *str)){
      str++;
    }
    tf_list_push(list, sp, str - sp);
    if(*str == '\0') break;
    str++;
  }
  return list;
}

/* Create a string by joining all elements of a list object. */
char *tf_strjoin(const tf_list_t list, char delim){
  assert(list);
  int num = tf_list_num(list);
  int size = num + 1;
  for(int i = 0; i < num; i++){
    size += TFLISTVALSIZ(list,i);
  }
  char *buf = tf_alloc(size);
  char *wp = buf;
  for(int i = 0; i < num; i++){
    if(i > 0) *(wp++) = delim;
    int vsiz;
    const char *vbuf = tf_list_val(list, i, &vsiz);
    memcpy(wp, vbuf, vsiz);
    wp += vsiz;
  }
  *wp = '\0';
  return buf;
}

/* Convert an integer to the string as binary numbers. */
int tf_num2strbin(uint64_t num, char *buf, int col, int fc){
  assert(buf);
  char *wp = buf;
  int len = sizeof(num) * 8;
  bool zero = true;
  while(len-- > 0){
    if(num & (1ULL << 63)){
      *(wp++) = '1';
      zero = false;
    } else if(!zero){
      *(wp++) = '0';
    }
    num <<= 1;
  }
  if(col > 0){
    if(col > sizeof(num) * 8) col = sizeof(num) * 8;
    len = col - (wp - buf);
    if(len > 0){
      memmove(buf + len, buf, wp - buf);
      for(int i = 0; i < len; i++){
        buf[i] = fc;
      }
      wp += len;
    }
  } else if(zero){
    *(wp++) = '0';
  }
  *wp = '\0';
  return wp - buf;
}

/* Convert a string to an integer. */
int64_t tf_atoi(const char *str){
  assert(str);
  while(*str > '\0' && *str <= ' '){
    str++;
  }
  int sign = 1;
  int64_t num = 0;
  if(*str == '-'){
    str++;
    sign = -1;
  } else if(*str == '+'){
    str++;
  }
  while(*str != '\0'){
    if(*str < '0' || *str > '9') break;
    num = num * 10 + *str - '0';
    str++;
  }
  return num * sign;
}

/* Convert a string with a metric prefix to an integer. */
int64_t tf_atoix(const char *str){
  assert(str);
  while(*str > '\0' && *str <= ' '){
    str++;
  }
  int sign = 1;
  if(*str == '-'){
    str++;
    sign = -1;
  } else if(*str == '+'){
    str++;
  }
  long double num = 0;
  while(*str != '\0'){
    if(*str < '0' || *str > '9') break;
    num = num * 10 + *str - '0';
    str++;
  }
  if(*str == '.'){
    str++;
    long double base = 10;
    while(*str != '\0'){
      if(*str < '0' || *str > '9') break;
      num += (*str - '0') / base;
      str++;
      base *= 10;
    }
  }
  num *= sign;
  while(*str > '\0' && *str <= ' '){
    str++;
  }
  if(*str == 'k' || *str == 'K'){
    num *= 1LL << 10;
  } else if(*str == 'm' || *str == 'M'){
    num *= 1LL << 20;
  } else if(*str == 'g' || *str == 'G'){
    num *= 1LL << 30;
  } else if(*str == 't' || *str == 'T'){
    num *= 1LL << 40;
  } else if(*str == 'p' || *str == 'P'){
    num *= 1LL << 50;
  } else if(*str == 'e' || *str == 'E'){
    num *= 1LL << 60;
  }
  if(num > INT64_MAX) return INT64_MAX;
  if(num < INT64_MIN) return INT64_MIN;
  return num;
}

/* Convert a string to a real number. */
double tf_atof(const char *str){
  assert(str);
  while(*str > '\0' && *str <= ' '){
    str++;
  }
  int sign = 1;
  if(*str == '-'){
    str++;
    sign = -1;
  } else if(*str == '+'){
    str++;
  }
  if(tf_strifwm(str, "inf")) return  ((float)3.40282346638528860e+38)*sign;
  if(tf_strifwm(str, "nan")) return nan("");
  long double num = 0;
  int col = 0;
  while(*str != '\0'){
    if(*str < '0' || *str > '9') break;
    num = num * 10 + *str - '0';
    str++;
    if(num > 0) col++;
  }
  if(*str == '.'){
    str++;
    long double fract = 0.0;
    long double base = 10;
    while(col < TFLDBLCOLMAX && *str != '\0'){
      if(*str < '0' || *str > '9') break;
      fract += (*str - '0') / base;
      str++;
      col++;
      base *= 10;
    }
    num += fract;
  }
  if(*str == 'e' || *str == 'E'){
    str++;
    num *= pow(10, tf_atoi(str));
  }
  return num * sign;
}

void tf_datetime_mmddhhmiss(char *str)
{
    time_t tt;
    struct tm *tm;
    tt = time(0);
    tm = localtime(&tt);

    sprintf(str,"%02d%02d%02d%02d%02d",
            tm->tm_mon+1,tm->tm_mday,tm->tm_hour, tm->tm_min,tm->tm_sec);
}

void tf_datetime(char *str)
{
    time_t tt;
    struct tm *tm;
    tt = time(0);
    tm = localtime(&tt);

    sprintf(str,"%04d%02d%02d%02d%02d%02d",
            tm->tm_year+1900, tm->tm_mon+1,tm->tm_mday,tm->tm_hour, tm->tm_min,tm->tm_sec);
}

/* rate controls (for implementing connect-limiting or karma) */
tf_rate_t rate_new(__u32 total, __u32 seconds, __u32 wait)
{
    tf_rate_t rt = (tf_rate_t) tf_alloc(sizeof(struct tf_rate_st));

    rt->total   = total;
    rt->seconds = seconds;
    rt->wait    = wait;
    rt->rate    = 0;

    return rt;
}

void tf_rate_free(tf_rate_t rt)
{
    tf_free(rt);
}

void tf_rate_reset(tf_rate_t rt)
{
    rt->time  = 0;
    rt->count = 0;
    rt->bad   = 0;
}

void tf_rate_add(tf_rate_t rt, __u32 count) {
    time_t now;
    now = time(0);

    /* rate expired */
    if(now - rt->time >= rt->seconds){
        rt->rate = rt->count*1.0/(now - rt->time);
        tf_rate_reset(rt);
    }

    rt->count += count;

    /* first event, so set the time */
    if(rt->time == 0)
        rt->time = now;

    /* uhoh, they stuffed up */
    if(rt->count >= rt->total)
        rt->bad = now;
}

int tf_rate_left(tf_rate_t rt) {
    /* if we're bad, then there's none left */
    if(rt->bad != 0)
        return 0;

    return rt->total - rt->count;
}

bool tf_rate_check(tf_rate_t rt) {
    /* not tracking */
    if(rt->time == 0)
        return true;

    /* under the limit */
    if(rt->count < rt->total){
        return true;
    }

    /* currently bad */
    if(rt->bad != 0) {
        /* wait over, they're good again */
        if(time(0) - rt->bad >= rt->wait) {
            tf_rate_reset(rt);
            return true;
        }
        /* keep them waiting */
        return false; 
    }

    /* they're inside the time, and not bad yet */
    return true;
}

int asc_to_bin(char *hex,char *dsp,int count)
{
    int i,offset, tmp;
    offset = 0;

    for(i=0;i<count;i++)
    {
        tmp = dsp[i*2];
        if(isalpha(tmp)) offset = (isupper(tmp)? 0x41:0x61);

        hex[i] = ((dsp[i * 2] <= 0x39) ? dsp[i * 2] - 0x30
                                       : dsp[i * 2] - offset + 10);
        hex[i] = hex[i] << 4;

        tmp = dsp[i*2+1];
        if(isalpha(tmp)) offset = (isupper(tmp)? 0x41:0x61);

        hex[i] += ((dsp[i * 2 + 1] <= 0x39) ? dsp[i * 2 + 1] - 0x30
                                       : dsp[i * 2 + 1] - offset + 10);
    }
    return count;
}

int bin_to_asc( char *dsp, char *hex, int count)
{
    int i;
    char ch;
    for(i = 0; i < count; i++)
    {
        ch = (hex[i] & 0xf0) >> 4;
        dsp[i * 2] = (ch > 9) ? ch + 0x41 - 10 : ch + 0x30;
        ch = hex[i] & 0xf;
        dsp[i * 2 + 1] = (ch > 9) ? ch + 0x41 - 10 : ch + 0x30;
    }
    return 2*count;
}

int bcd_to_asc(char *dst,char *src,int len)
{
    int tmp_in,dstlen = 0;
    unsigned char   temp;

    for( tmp_in = 0; tmp_in < len ; tmp_in ++ )
    {
        temp = src[tmp_in] & 0xF0;
        temp = temp >> 4;

        dst[2*tmp_in] = temp + 0x30;

        temp = src[tmp_in] & 0x0F;
        dst[2*tmp_in+1] = temp + 0x30;
    }
    dstlen = len*2;
    return dstlen;
}

int asc_to_bcd(char *dst,char *src,int len)
{
    int tmp_in;
    unsigned char temp,d1,d2;

    for( tmp_in = 0; tmp_in < len ; )
    {
        temp = src[tmp_in];
        d1 = temp - 0x30;

        tmp_in++;

        if (tmp_in>=len)
        {
            d2=0x00;
        }
        else
        {
            temp = src[tmp_in];
            d2 = temp - 0x30;
        }

        tmp_in++;
        dst[(tmp_in-2)/2] = (d1<<4) | d2;
    }

    return (tmp_in+1)/2;
}
