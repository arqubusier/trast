/*
 * Returns the number of digits needed to represent a
 * string in decimal.
 */
#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

#define SHA1_LEN 20
#define BASE64_PAD(len) ((3-len%3)%3)
#define BASE64_LEN(len) (((len + BASE64_PAD(len))/3)*4)

typedef unsigned int uint;
int n_digits(int i);

/*
 * copies a string excluding the null character.
 * At most limit chars will be copied.
 */
size_t strcpy_limit(char* dst, const char* src, size_t limit);

size_t percent_encode_size(const char* src);
void percent_encode(char *dst, const char *src);
size_t base64_pad_len(const int src_len);
void base64_pad(char* src, const int src_len, const int pad_len);
void alpha_num_rand(char* buf, const int buf_len);
void base64_encode(char* dst, size_t dst_len,
                   const unsigned char* src, size_t src_len);
void print_hex(unsigned char* buf, uint buf_len);

#endif /* UTIL_H */
