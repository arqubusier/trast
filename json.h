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
    JSON_PARSER_FAULT = -3 //the parser made an illegal action
};

enum json_match{
    JSON_NO_MATCH = -1,
    JSON_MATCH = -2
    /* positive values used internally
     * to represent a match upto a
     * certain level */
};

typedef struct {
    const char **key_addr;
    size_t max_level;
    int match_upto_level;
    char *storage;
    size_t storage_sz;
    struct json_str_t *next;
} json_str_t;

typedef struct {
    const char **key_addr;
    size_t max_level;
    int match_upto_level;
    int *storage;
    struct json_int_t *next;
} json_int_t;


#define JSON_LIST_EMPTY NULL


typedef struct {
	unsigned int level; //current address level
    size_t buff_offset;
    size_t key_offset;
    int state;
    json_int_t *ints;
    size_t n_ints;
    json_str_t *strs;
    size_t n_strs
    json_int_t *int_matches;
    json_str_t *str_matches;
    bool should_find_matches;
} json_parser;

void json_ints_init(json_val_int_t *values, const char **keys_addrs[],
                    int *storages, size_t n_values);
void json_strs_init(json_val_int_t *values, const char **keys_addrs[],
                    int *storages, size_t n_values);

void json_parser_init(json_parser *parser, json_int_t *ints, size_t n_ints,
                      json_str_t *strs, size_t n_strs);

int json_parse(json_parser *parser, const char *input_buff, size_t buff_sz);

#ifdef __cplusplus
}
#endif

#endif /* __JSON_H_ */
