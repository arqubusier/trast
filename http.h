#ifndef HTTP_H
#define HTTP_H


int substr_init(const int substr_id);
int substr_free(const int substr_id);
int substr_set_param_int(const int substr_id,const int param, int val);
int substr_set_param_str(const int substr_id,const int param, char* str, size_t str_len);
int substr_assemble(const int substr_id);
int substr_combine(int* substr_ids);

#endif HTTP_H /* HTTP_H */
