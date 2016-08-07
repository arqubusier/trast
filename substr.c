#include "substr.h"
#include "substr_conf.h"

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
    int param_id=0;
    for (; param_id < substr_n_params[substr_id]; param_id++){
        substrs_param_szs[substr_id][param_id] = 0;
    }
    return 0;
}

//TODO: add deep free
int substr_free(const int substr_id){
    free(substrs[substr_id]);
    return 0;
}

size_t substr_alloc_num(const int substr_id, const int param_id, long val){
    /* Reallocate if the buffer is to small */
    size_t new_size = n_digits(val);
    size_t old_size = PARAM_SZ(substr_id,param_id);
    if (old_size ==  0){
        new_size = new_size + PARAM_DEFAULT_LEN;
        SET_PARAM_SZ(substr_id, param_id, new_size);
        substrs[substr_id][param_id] = malloc(new_size);        
    }
    else if (new_size >= old_size){
        new_size = new_size + PARAM_DEFAULT_LEN;
        SET_PARAM_SZ(substr_id, param_id, new_size);
        substrs[substr_id][param_id] = realloc(
                (void*)substrs[substr_id][param_id], new_size);        
    }

    return new_size;
}

int substr_set_param_long(const int substr_id, const int param_id, long val){
    size_t new_size = substr_alloc_num(substr_id, param_id, val);

    sprintf(substrs[substr_id][param_id], "%ld", val);  
    substrs[substr_id][param_id][new_size] = '\0';

    return 0;
}

int substr_set_param_int(const int substr_id, const int param_id, int val){
    size_t new_size = substr_alloc_num(substr_id, param_id, val);

    sprintf(substrs[substr_id][param_id], "%d", val);  
    substrs[substr_id][param_id][new_size] = '\0';

    return 0;
}

int substr_set_param_str(const int substr_id, const int param_id,
                         const char* str){
    size_t str_len = strlen(str);
    size_t new_size = str_len;
    size_t old_size = PARAM_SZ(substr_id, param_id);

    if (old_size ==  0){
        new_size = new_size + PARAM_DEFAULT_LEN;
        SET_PARAM_SZ(substr_id, param_id, new_size);
        substrs[substr_id][param_id] = malloc(new_size);        
    }
    else if (new_size >= old_size){
        new_size = new_size + PARAM_DEFAULT_LEN;
        SET_PARAM_SZ(substr_id, param_id, new_size);
        substrs[substr_id][param_id] = realloc(
                (void*)substrs[substr_id][param_id], new_size);        
    }
    
    strncpy(substrs[substr_id][param_id], str, str_len);
    substrs[substr_id][param_id][str_len] = '\0';

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

size_t substr_len(const int substr_id){
    size_t total_size = 0;
    int param_id = 0;

    for (; param_id < substr_n_params[substr_id]; param_id++){
        size_t max = PARAM_SZ(substr_id, param_id);
        total_size += strnlen(substrs[substr_id][param_id], max);
        printf("par %d total size %d\n", param_id, total_size);
    }
    return total_size;
}

/*
 * dst must have at least one character
 */
int substr_assemble(char* dst, const int substr_id, size_t dst_sz){
    char **substr = substrs[substr_id];
    int *param_szs = substrs_param_szs[substr_id];

    int param_id = 0;
    int offs = 0;
    for (; param_id < substr_n_params[substr_id] &&
                offs<dst_sz; param_id++){
        offs += strcpy_limit(&dst[offs], substr[param_id], param_szs[param_id]); 
    }

    dst[offs] = '\0';
    return 0;
}

int substr_combine(int* substr_ids, unsigned int n_ints){
    unsigned int i = 0;
    size_t total_size = 0;

    for (;i < n_ints; ++i){
        total_len += substr_param_len(substr_ids[i]);
    }

    char res[total_len + 1];

    unsigned int offs = 0;
    unsigned int max = 0;
    for (;i < n_ints; ++i){
        max = PARAM_SZ(substr_ids[i]);
        offs += strcpy_limit(&res[offs], &substr_ids[i], max);
    }
    res[total_len] = '\0';
    return 0;
}

const char* substr_get_param(const int substr_id, const int param_id){
    return substrs[substr_id][param_id];
}
