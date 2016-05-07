#include "http.h"
#include "http_conf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"


int substr_n_params[] = SUBSTR_IDS;

#define SUBSTRS_LEN  sizeof(substr_n_params)/sizeof(int)
#define SUBSTR_N_PARAMS(substr_id) substr_n_params[substr_id]
#define PARAM_DEFAULT_LEN 5

char **substrs[SUBSTRS_LEN];
int *substrs_param_szs[SUBSTRS_LEN];

#define PARAM_SZ(substr_id, param_id) substrs_param_szs[substr_id][param_id]
#define SET_PARAM_SZ(substr_id, param_id, new) substrs_param_szs[substr_id][param_id] = new

int substr_init(const int substr_id){
    substrs[substr_id] = malloc(substr_n_params[substr_id]*sizeof(char*));
    substrs_param_szs[substr_id] = malloc(substr_n_params[substr_id]*sizeof(int));
    memset(substrs_param_szs[substr_id], 0, SUBSTR_N_PARAMS(substr_id)*sizeof(int));
    return 0;
}

//TODO: add deep free
int substr_free(const int substr_id){
    free(substrs[substr_id]);
    return 0;
}

int substr_set_param_int(const int substr_id, const int param_id, int val){
    /* Reallocate if the buffer is to small */
    size_t new_size = n_digits(val);
    size_t old_size = PARAM_SZ(substr_id,param_id);
    if (new_size > old_size){
        new_size = new_size + PARAM_DEFAULT_LEN;
        SET_PARAM_SZ(substr_id, param_id, new_size);
        substrs[substr_id][param_id] = realloc(
                (void*)substrs[substr_id][param_id], new_size);        
    }

    sprintf(substrs[substr_id][param_id], "%d", val);  
    return 0;
}

int substr_set_param_str(const int substr_id, const int param_id,
                         char* str, size_t str_len){
    size_t new_size = str_len;
    size_t old_size = PARAM_SZ(substr_id, param_id);
    if (new_size > old_size){
        new_size = new_size + PARAM_DEFAULT_LEN;
        SET_PARAM_SZ(substr_id, param_id, new_size);
        substrs[substr_id][param_id] = realloc(
                (void*)substrs[substr_id][param_id], new_size);        
    }
    
    strncpy(substrs[substr_id][param_id], str, str_len);
    return 0;
}

size_t substr_size(const int substr_id){
    size_t total_size = 0;
    int param_id = 0;
    for (; param_id < substr_n_params[substr_id]; param_id++){
        total_size += PARAM_SZ(substr_id, param_id);
    }
    return total_size;
}

/*
 * dst must have at least one character
 */
int substr_assemble(char* dst, const int substr_id, size_t dst_len){
    char **substr = substrs[substr_id];
    int *param_szs = substrs_param_szs[substr_id];

    int param_id = 0;
    int offs = 0;
    for (; param_id < substr_n_params[substr_id] &&
                offs<dst_len; param_id++){
        offs += strcpy_limit(&dst[offs], substr[param_id], param_szs[param_id]); 
    }

    dst[offs] = '\0';
    return 0;
}

int substr_combine(int* substr_ids){
    return 0;
}
