#ifndef HTTP_H
#define HTTP_H


void substr_init(int substr_id);
void substr_free(int substr_id);
int substr_set_param_int(int substr_id, int param, int val);
int substr_set_param_str(int substr_id, int param, char* str, size_t str_len);

#endif HTTP_H /* HTTP_H */
