#ifndef SUBSTR_H
#define SUBSTR_H

#include <stddef.h>

#include "util.h"


int substr_init(const int substr_id);
int substr_free(const int substr_id);
int substr_set_param_int(const int substr_id,const int param, int val);
int substr_set_param_str(const int substr_id,const int param, const char* str);
size_t substr_size(const int substr_id);
int substr_assemble(char* dst, const int substr_id, size_t dst_len);
int substr_combine(int* substr_ids);

#endif/* SUBSTR_H */
