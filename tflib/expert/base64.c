#include "tflib.h"
#include "base64.h"

static const unsigned char BIN2BASE64[65]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const unsigned char BASE642BIN[256] = {
    96, 99, 99, 99, 99, 99, 99, 99, 99, 99, 98, 99, 99, 98, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 62, 99, 99, 99, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 99, 99, 99, 97, 99, 99,
    99,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 99, 99, 99, 99, 99,
    99, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99
  };

int outputbase64(unsigned char *out,const unsigned char * data,short usecrlf,int *nextline)
{
    int enc_len = 0;
    out[enc_len++] = BIN2BASE64[data[0] >> 2];
    out[enc_len++] = BIN2BASE64[((data[0]&3)<<4) | (data[1]>>4)];
    out[enc_len++] = BIN2BASE64[((data[1]&15)<<2) | (data[2]>>6)];
    out[enc_len++] = BIN2BASE64[data[2]&0x3f];

    if (++(*nextline) > 18) { // 76 columns
//        if (usecrlf)
//            out[enc_len++] = '\r';
//        out[enc_len++] = '\n';
        *nextline = 0;
    }
    return enc_len;
}

int encode_base64(unsigned char *dst,unsigned char *src,int length)
{
    int   i,encode_len,next_line,save_count;
    short use_crlf = 1;
    unsigned char save_triple[3];
    unsigned char *data = src;

    encode_len = next_line = save_count = 0;

    if (length == 0)
      return 0;

    while (save_count < 3) {
        save_triple[save_count++] = *data++;
        if (--length == 0)
        return encode_len;
    }

    encode_len = outputbase64(dst,save_triple,use_crlf,&next_line);

    for (i = 0; i+2 < length; i += 3)
    {
        encode_len += outputbase64(dst+encode_len,data+i,use_crlf,&next_line);
    }

    save_count = length - i;
    switch (save_count) {
    case 2 :
        save_triple[0] = data[i++];
        save_triple[1] = data[i];
        break;
    case 1 :
        save_triple[0] = data[i];
    }

    data = dst+encode_len;
    switch (save_count) {
    case 1 :
        *data++ = BIN2BASE64[save_triple[0] >> 2];
        *data++ = BIN2BASE64[(save_triple[0]&3)<<4];
        *data++ = '=';
        *data   = '=';
        break;

    case 2 :
        *data++ = BIN2BASE64[save_triple[0] >> 2];
        *data++ = BIN2BASE64[((save_triple[0]&3)<<4) | (save_triple[1]>>4)];
        *data++ = BIN2BASE64[((save_triple[1]&15)<<2)];
        *data   = '=';
    }
    encode_len += 4;
    return encode_len;
}

int decode_base64(unsigned char *dst,unsigned char *src,int len)
{
    short perfect_decode = 1;
    int   decode_size,quad_position =0;
    unsigned char value;
    const unsigned char *cstr = src;
    unsigned char *out = dst;

    decode_size = 0;
    for (;;) 
    {
        value = BASE642BIN[(unsigned char)*cstr++];
        switch (value) {
        case 96 : // end of string
            return decode_size;

        case 97 : // '=' sign
            if (quad_position == 3 || (quad_position == 2 && *cstr == '=')) 
            {
                quad_position = 0;  // Reset this to zero, as have a perfect decode
                return decode_size; // Stop decoding now as must be at end of data
            }
            perfect_decode = 0;  // Ignore '=' sign but flag decode as suspect
            break;
        case 98 : // CRLFs
            break;  // Ignore totally

        case 99 :  // Illegal characters
            perfect_decode = 0;  // Ignore rubbish but flag decode as suspect
            break;

        default : // legal value from 0 to 63
            switch (quad_position) {
            case 0 :
                out[decode_size]    = (unsigned char)(value << 2);
                break;
            case 1 :
                out[decode_size++] |= (unsigned char)(value >> 4);
                out[decode_size]    = (unsigned char)((value&15) << 4);
                break;
            case 2 :
                out[decode_size++] |= (unsigned char)(value >> 2);
                out[decode_size]    = (unsigned char)((value&3) << 6);
                break;
            case 3 :
                out[decode_size++] |= (unsigned char)value;
                break;
            }
        quad_position = (quad_position+1)&3;
        }
    }

    return decode_size;
}
