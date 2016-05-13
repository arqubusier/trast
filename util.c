#include "util.h"
#include <string.h>
#include <stdio.h>
#include "esp/hwrand.h"
#include "sys/types.h"

const char BASE64_ALPHABET0[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const char ALPHA_NUM[] = BASE64_ALPHABET;
const int  ALPHA_NUM_LEN = 62;

int n_digits(int i){
    if (i < 0) i = -i + 1;
    if (i <         10) return 1;
    if (i <        100) return 2;
    if (i <       1000) return 3;
    if (i <      10000) return 4;
    if (i <     100000) return 5;
    if (i <    1000000) return 6;      
    if (i <   10000000) return 7;
    if (i <  100000000) return 8;
    if (i < 1000000000) return 9;
    return 10;
}

size_t strcpy_limit(char* dst, const char* src, size_t limit){
    int i = 0;
    for (;i<limit && src[i] != '\0'; i++){
        dst[i] = src[i];
    }
    return i;
};

size_t percent_encode_size(const char* src){
    size_t src_sz = strlen(src);
    int i=0;
    size_t total_size=0;
    for (; i < src_sz; i++){
        char c = src[i];
        if ((c >= 0x30 && c <= 0x39) || 
                (c >= 0x41 && c <= 0x5A) ||
                (c >= 0x61 && c <= 0x7A) ||
                c==0x2D || c==0x2E || c == 0x5F || c==0x7E){
            total_size += 1;
        }
        else{
            total_size += 3;
        }
    }

    return total_size;
}

void percent_encode(char *dst, const char *src){
    size_t src_sz = strlen(src);
    int i=0, offs=0, total_size=0;
    for (; i < src_sz; i++){
        char c = src[i];
        if ((c >= 0x30 && c <= 0x39) || 
                (c >= 0x41 && c <= 0x5A) ||
                (c >= 0x61 && c <= 0x7A) ||
                c==0x2D || c==0x2E || c == 0x5F || c==0x7E){
            dst[offs] = src[i];
            offs += 1;
        }
        else{
            sprintf(&dst[offs], "%d2x", src[i]);
            offs += 3;
        }
    }
}

size_t base64_pad_len(const int src_size){
    size_t total_size = src_size + src_size%3;
    total_size = (total_size/3)*4;
}

/*
 * Convert a byte array to a base64 array. This assumes that src is a multiple
 * of 3 and that dst has the size according to the base64 encoding.
 *
 *    -------------------------------------------------------------------------
 *  ||b  y  t  e  0  0 | b  y||t  e  0  1 | b  y  t  e||0  2 | b  y  t  e  0  3||
 *    -------------------------------------------------------------------------
 *    0  1  2  3  4  5   6  7  0  1  2  3   4  5  6  7  0  1   2  3  4  5  6  7 
 */

void base64_encode(unsigned char* dst, size_t dst_len,
                   unsigned char* src, size_t src_len){
    int dst_offs = 0, src_offs = 0;
    if (dst_len > 0){
        dst[dst_len-2] = '=';
        dst[dst_len-1] = '=';
        dst[dst_len-0] = '=';
    }

    for(; src_offs < src_len; src_offs+=3, dst_offs+=4){
        dst[dst_offs] = src[src_offs] >> 2;
        dst[dst_offs + 1] = (src[src_offs] << 6) & (src[src_offs] >> 4);
        dst[dst_offs + 2] = (src[src_offs] << 4) & (src[src_offs] >> 6);
        dst[dst_offs + 3] = src[src_offs] >> 2 & 0xC0;
    }
}

void base64_pad(char* src, const int src_len, const int pad_len){
    int i=src_len;
    for (; i < src_len + pad_len; i++){
        src[i] = '=';
    }
}

void alpha_num_rand(char* buf, const int buf_len){
    hwrand_fill((uint8_t *)buf, buf_len);
    int i = 0;
    for (;i < buf_len; i++){
        buf[i] =  ALPHA_NUM[buf[i] % ALPHA_NUM_LEN];
    }
}
