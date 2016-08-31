#ifndef __JSON_H_
#define __JSON_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum json_res {
    JSON_CONTINUE = 1,
    JSON_DONE = 0,
	JSON_ERROR_PARSING = -1,
	JSON_ERROR_STORAGE_SIZE = -2,
};

enum json_match{
    JSON_NO_MATCH = -1,
    JSON_MATCH = -2
    /* positive values used internally
     * to represent a match upto a
     * certain level */
};

typedef struct json_str_t json_str_t;
struct json_str_t {
    //must have at least one level
    const char **key_addr;
    int match_upto_level;
    char *storage;
    size_t storage_sz;
    json_str_t *next;
};

typedef struct json_int_t json_int_t;
struct json_int_t{
    //must have at least one level
    const char **key_addr;
    int match_upto_level;
    int storage;
    json_int_t *next;
};


#define JSON_LIST_EMPTY NULL


typedef struct {
	unsigned int level; //current address level
    size_t buff_offset;
    size_t key_offset;
    int state;
    json_int_t *ints;
    size_t n_ints;
    json_str_t *strs;
    size_t n_strs;
    json_int_t *int_matches;
    json_str_t *str_matches;
    int should_find_matches;
} json_parser;

void json_ints_init(json_int_t *values, const char **keys_addrs[],
                    int *storages, size_t n_values);
void json_strs_init(json_str_t *values, const char **keys_addrs[],
                    char **storages, size_t n_values);

void json_parser_init(json_parser *parser, json_int_t *ints, size_t n_ints,
                      json_str_t *strs, size_t n_strs);

int json_parse(json_parser *parser, const char *input_buff, size_t buff_sz);

#ifdef __cplusplus
}
#endif

#endif /* __JSON_H_ */
