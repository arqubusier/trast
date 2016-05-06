#include "http.h"
#include "http_conf.h"

#include <stdio.h>
#include <stdlib.h>

#include "util.h"

char **substrs[SUB_STR_N_PARAMS_LEN];
int *substrs_param_szs[SUB_STR_N_PARAMS_LEN];

#define PARAM_SZ(substr_id, param_id) sub_strs_param_szs[substr_id][param_id]
#define SET_PARAM_SZ(substr_id, param_id, new) sub_strs_param_szs[substr_id][param_id] = new
int substr_init(const int substr_id){
    substrs[substr_id] = malloc(SUBSTR_SZ(substr_id)*subs*sizeof(char*));
    substrs_param_szs[substr_id] = malloc(SUBSTR_SZ(substr_id)*sizeof(int));
    memset(substr_param_szs[substr_id], 0, SUBSTR_N_PARAMS[substr_id]);
    return 0
}

//TODO: add deep free
int substr_free(const int substr_id){
    free(substrs[substr_id])
    return 0;
}

int substr_set_param_int(const int substr_id, const int param_id, int val){
    /* Reallocate if the buffer is to small */
    int new_size = n_digits(val);
    int old_size = PARAM_SZ(substr_set_param_id,param);
    if (new_size > old_size){
        new_size = new_size + PARAM_DEFAULT_LEN;
        SET_PARAM_SZ(substr_id, param_id, new_size);
        substr[substr_id][param_id] = realloc(
                (void*)substr[substr_id][param_id], new_size);        
    }

    sprintf(substr[substr_id][param], "%d", val);  
    
    return 0;
}

int substr_set_param_str(const int substr_id, const int param,
                         char* str, size_t str_len){
    //check how long
    
    //(re)allocate appropriate size
    
    //insert val
    return 0;
}

int substr_assemble(const int substr_id){

    return 0;
}

int substr_combine(int* substr_ids){

    return 0;
}
