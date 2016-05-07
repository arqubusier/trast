#include "util.h"

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
