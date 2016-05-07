#include "util.h"
#include <string.h>
#include <stdio.h>

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
    int i=0, total_size=0;
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
