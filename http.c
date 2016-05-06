#include "http.h"
#include "http_conf.h"

#include <stdio.h>


char **sub_strs[SUB_STR_N_PARAMS_LEN];

void substr_init(int substr_id){

}

void substr_free(int substr_id){
    ;

}

int substr_set_param_int(int substr_id, int param, int val){
    //check how long
    
    //(re)allocate appropriate size
    
    //insert val
    ;
}

int substr_set_param_str(int substr_id, int param, char* str, size_t str_len){
    //check how long
    
    //(re)allocate appropriate size
    
    //insert val
    ;
}
