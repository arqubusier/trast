#include "json.h"


enum json_state{
    JSON_STATE_START,
    JSON_STATE_BEGIN_VAL_OR_KEY,
    JSON_STATE_INT_ELEM,
    JSON_STATE_TRUE_ELEM,
    JSON_STATE_FALSE_ELEM,
    JSON_STATE_NULL_ELEM,
    JSON_STATE_KEY_OR_STR,
    JSON_STATE_KEY_OR_STR_ESCAPE,
    JSON_STATE_KEY_OR_STR_END,
    JSON_STATE_TYPE,
    JSON_STATE_INT,
    JSON_STATE_INT,
    JSON_STATE_TRUE,
    JSON_STATE_FALSE,
    JSON_STATE_STR,
    JSON_STATE_STR_ESCAPE,
    JSON_STATE_END_VAL
}

/*----------------------------------------------------------------------------*
 * State handling functions
 *----------------------------------------------------------------------------*/
const char *const TRUE_VAL[] = "true";
const char *const FALSE_VAL[] = "false";
const char *const NULL_VAL[] = "null";
const size_t TRUE_VAL_LEN = 4;
const size_t FALSE_VAL_LEN = 5;
const size_t NULL_VAL_LEN = 4;

inline bool is_special(const size_t offs, const char c, int state){
    return ;
        
}

int handle_begin_value_or_key(json_parser *parser, const char c){
    switch (c){
        case '{': case '[':
            parser->level++;
            break;
        case '"':
            parser->state = JSON_STATE_KEY_OR_STR;
            break;
        case '0' ... '9':
            parser->state = JSON_STATE_INT_ELEM;
            break;
        case 't', 'f', 'n':
            parser->state = JSON_STATE_SPECIAL_ELEM;
            break;
        //skip White spaces
        case ' ': case '\t': case '\r': case '\n': case: '\t':
            break;
        default:
            return JSON_ERROR_PARSING;
    }
    return JSON_CONTINUE;
}

int handle_int_elem(json_parser *parser, const char c){
    switch (c){
        //TODO: add matching inside arrays
        case '0' ... '9':
            break;
        case ']':
            parser->level--;
            parser->state = JSON_STATE_VAL_END;
            parser->key_offset = 0;
            break;
        case ',':
            parser->state = JSON_STATE_BEGIN_VAL_OR_KEY;
            parser->key_offset = 0;
            break;
        default:
            return JSON_ERROR_PARSING;
    }
    return JSON_CONTINUE;

}

int handle_special_elem(json_parser *parser, const char c){
    switch (c){
        case ']':
            parser->level--;
            parser->state = JSON_STATE_VAL_END;
            parser->key_offset = 0;
            break;
        case ',':
            parser->state = JSON_STATE_BEGIN_VAL_OR_KEY;
            parser->key_offset = 0;
            break;
        default:
            //check if c is part of a special literal
            if !((state == JSON_STATE_TRUE_ELEM && offs < TRUE_VAL_LEN
                    && c == TRUE_VAL[offs])
                  || (state == JSON_STATE_NULL_ELEM && offs < NULL_VAL_LEN
                    && c == NULL_VAL[offs])
                  || (state == JSON_STATE_FALSE_ELEM && offs < FALSE_VAL_LEN
                    && c == FALSE_VAL[offs]))
                return JSON_ERROR_PARSING;
            parser->key_offset++;
    }
    return JSON_CONTINUE;
}

//TODO: ADD SAVE TO STORAGE == ADD ARRAY SUPPORT
int handle_key_or_str(json_parser *parser, const char c){
    size_t i;
    json_int_t *prev_int_match = int_matches;
    json_str_t *prev_str_match = str_matches;
    json_int_t *int_match;
    json_str_t *str_match;

    if (parser->state == JSON_STATE_KEY_OR_STR && c == '\\'){
        parser->state = JSON_STATE_KEY_OR_STR_ESCAPE;
        parser->key_offset++;
    }else if (parser->state == JSON_STATE_KEY_OR_STR && c == '\"'){
        parser->state = JSON_STATE_KEY_OR_STR_END;
        //mark matches
        for (int_match = parser->int_matches; int_match != JSON_LIST_EMPTY;
                int_match = int_match->next){
            int_match->match_upto_level++;
        }

        parser->key_offset = 0;
    }else{
        //fill the list of matches
        if (parser->should_find_matches){
            if (parser->n_ints > 0){
                i = parser->n_ints;
                parser->int_matches = JSON_LIST_EMPTY;
                do {
                    --i;
                    // Note that keys must contain at least one char
                    // namely '\0'

                    int_match = &int_matches[i];
                    if ((int_match->match_upto_level == parser->level - 1)
                         && (c == int_match->key_addr[parser->level][0])){
                        int_match->next = parser->int_matches;
                        parser->int_matches = int_match;
                    }
                } while(i != 0); 
            }

            if (parser->n_strs > 0){
                i = parser->n_strs;
                parser->strs_matches = JSON_LIST_EMPTY;
                do {
                    --i;
                    // Note that keys must contain at least one char
                    // namely '\0'

                    str_match = &str_matches[i];
                    if ((str_match->match_upto_level == parser->level - 1)
                         && (c == str_match->key_addr[parser->level][0])){
                        str_match->next = parser->str_matches;
                        parser->str_matches = str_match;
                    }
                } while(i != 0); 
            }
        }
        //filter out key addresses that don't match
        else{
            int_match = parser->int_matches;
            prev_int_match = int_match;
            for (;int_match != JSON_LIST_EMPTY;
                   int_match = int_match->next){
                if (c != int_match->key_addr[parser->level][key_offset])
                    if (prev_int_match == parser->int_matches)
                        parser->int_matches = int_match->next;
                    else
                        prev_int_match->next = int_match->next;
                else
                    prev_int_match = int_match;
            }
        }

        parser->key_offset++;
    }

    return JSON_CONTINUE;
}

int handle_key_or_str_end(json_parser *parser, const char c){
    switch (c){
        //TODO ADD ARRAY SUPPORT
        //skip White spaces
        case ' ': case '\t': case '\r': case '\n': case: '\t':
            break;
        //str array element
        case ',':
            parser->state = JSON_STATE_BEGIN_VAL_OR_KEY;
            parser->key_offset = 0;
            break;
        //str array element
        case ']':
            parser->state = JSON_STATE_END_VAL;
            parser->key_offset = 0;
            break;
        case ':':
            parser->state = JSON_STATE_TYPE;
            parser->key_offset = 0;
            break;
        default:
            return JSON_ERROR_PARSING;
    }

    return JSON_CONTINUE;
}

int handle_type(json_parser *parser, const char c){
    switch (c){
        //TODO ADD SPECIAL STORAGE SUPPORT
        //TODO ADD ARRAY SUPPORT
        //skip White spaces
        case ' ': case '\t': case '\r': case '\n': case: '\t':
            break;
        case '{': case '[':
            parser->state = JSON_STATE_BEGIN_VAL_OR_KEY;
            break;
        case '0' ... '9':
            parser->state = JSON_STATE_INT;
            break;
        case 't':
            parser->state = JSON_STATE_TRUE;
            break;
        case 'f':
            parser->state = JSON_STATE_FALSE;
            break;
        case 'n':
            parser->state = JSON_STATE_NULL;
            break;
        case '\"':
            parser->state = JSON_STATE_STR;
            break;
        default:
            return JSON_ERROR_PARSING;
    }

    return JSON_CONTINUE;
}

int handle_int(json_parser *parser, const char c){
    if (parser->int_matches != JSON_LIST_EMPTY){
        int digit = c_to_int(c);
        if (tmp == -1)
            return JSON_ERROR_PARSING;
        parser->int_matches->storage *= 10;
        parser->int_matches->storage += digit;
    }
    else
        return handle_end_val(parser, c);
}

int handle_special(json_parser *parser, const char c){
    if ((state == JSON_STATE_TRUE && offs < TRUE_VAL_LEN
            && c == TRUE_VAL[offs])
          || (state == JSON_STATE_NULL && offs < NULL_VAL_LEN
            && c == NULL_VAL[offs])
          || (state == JSON_STATE_FALSE && offs < FALSE_VAL_LEN
            && c == FALSE_VAL[offs]))
        return JSON_CONTINUE;
    else
        return handle_end_val(parser, c);

}

int handle_str(json_parser *parser, const char c){
    if (parser->state != JSON_STATE_STR_ESCAPE && c == '\\')
        parser->state = JSON_STATE_STR_ESCAPE;
    else if (parser->state != JSON_STATE_STR_ESCAPE && c == '\"'){
        if (parser->key_offset + 1 > str->storage_sz)
            return JSON_ERROR_STORAGE_SIZE;
        parser->state = JSON_STATE_END_VAL;
        parser->str_matches->storage[parser->key_offset] = '\0';
        parser->key_offset = 0;
    }
    else if (parser->str_matches != JSON_LIST_EMPTY){
        json_str_t *str = parser->str_matches;
        if (parser->key_offset + 1 > str->storage_sz)
            return JSON_ERROR_STORAGE_SIZE;
        str[key_offset] = c;
        parser->key_offset++;
    }

    return JSON_CONTINUE;
}

int handle_end_val(json_parser *parser, const char c){
    switch (c){
        //skip White spaces
        case ' ': case '\t': case '\r': case '\n': case: '\t':
            break;
        case '}':
            parser->level--;
            if (level == 0)
                return JSON_DONE;
            break;
        case ']':
            parser->level--;
            break;
        case ',':
            parser->state = JSON_STATE_BEGIN_VAL_OR_KEY;
            break;
        default:
            return JSON_ERROR_PARSING;
    }

    return JSON_CONTINUE;
}

int handle_start(json_parser *parser, const char c){
    switch (c){
        //skip White spaces
        case ' ': case '\t': case '\r': case '\n': case: '\t':
            break;
        case '{':
            parser->state = JSON_STATE_BEGIN_VAL_OR_KEY
            break:
        default:
            return JSON_ERROR_PARSING;
    }
    return JSON_CONTINUE;
}
//TODO ADD OPTIMISATIONS DURING KEY MATCHING
int json_parse(json_parser *parser,
               const char *input_buff, size_t buff_sz);
	int i;

    //The parser looks at one char at a time.
    //the character is only advanced by this foor loop.
	for (; parser->buff_offset < buff_sz && js[parser->buff_offset] != '\0';
           parser->buff_offset++) {
		char c;
        int res;

		c = js[parser->offset];


        switch (parser->state){
            case JSON_STATE_BEGIN_VAL_OR_KEY:
                res = handle_begin_val_or_key(parser, c); break;
            case JSON_STATE_INT_ELEM:
                res = handle_int_elem(parser, c); break;
            case JSON_STATE_TRUE_ELEM: case JSON_STATE_FALSE_ELEM:
                    case JSON_STATE_NULL_ELEM;
                res = handle_special_elem(parser, c); break;
            case JSON_STATE_KEY_OR_STR: case JSON_STATE_KEY_OR_STR_ESCAPE:
                res = handle_key_or_str(parser, c); break;
            case JSON_STATE_KEY_OR_STR_END:
                res = handle_key_or_str_end(parser, c); break;
            case JSON_STATE_TYPE:
                res = handle_type(parser, c); break;
            case JSON_STATE_INT:
                res = handle_int(parser, c); break;
            case JSON_STATE_TRUE: case JSON_STATE_FALSE: case JSON_STATE_NULL;
                res = handle_special(parser, c); break;
            case JSON_STATE_STR: case JSON_STATE_ESCAPE:
                res = handle_str(parser, c); break;
            case JSON_STATE_END_VAL:
                res = handle_end_val(parser, c); break;
            case JSON_STATE_START:
                res = handle_start(parser, c); break;

            //parsing stop if we are done or if there was an error
            if (res <= 0)
                return res;
        }
	}

    //ran out of buffer, parser is either done or needs more input
    if (level == 0)
        return JSON_PARSING_DONE;
    return JSON_CONTINUE;
}

void json_parser_init(json_parser *parser, json_int_t *ints, size_t n_ints,
                      json_str_t *strs, size_t n_strs){
	parser->level = 0;
	parser->buff_offset = 0;
    parser->state = JSON_STATE_VAL;
    parser->int_matches = JSON_LIST_EMPTY;
    parser->n_ints = n_ints;
    parser->str_matches = JSON_LIST_EMPTY;
    parser->n_strs = n_strs;
}

void json_ints_init(json_int_t *values, const char **keys_addrs[],
                    int *storages, size_t n_values){
    size_t i;
    if (n_vals == 0)
        return;

    json_int_t *val; 
    for(i=0; i<n_vals - 1; ++i){
        val = &values[i];
        val->key_addr = key_addrs[i];
        val->storage = storages[i];
        val->max_levels = 
        val->match =0;
        val->next = &values[i+1];
    }

    /* NULL marks the end of the chained list */
    i = n_vals - 1;
    val = &values[i];
    val->key_addr = key_addrs[i];
    val->storage = storages[i];
    val->max_levels = 
    val->match = 0;
    val->next = NULL;
}

void json_str_init(json_str_t *values, const char **keys_addrs[],
                    int *storages, size_t n_values){
    size_t i;
    if (n_vals == 0)
        return;

    json_str_t *val; 
    for(i=0; i<n_vals - 1; ++i){
        val = &values[i];
        val->key_addr = key_addrs[i];
        val->storage = storages[i];
        val->storage_sz = 
        val->max_levels = 
        val->match = 0;
        val->next = &values[i+1];
    }

    /* NULL marks the end of the chained list */
    i = n_vals - 1;
    val = &values[i];
    val->key_addr = key_addrs[i];
    val->storage = storages[i];
    val->storage_sz = 
    val->max_levels = 
    val->match = 0;
    val->next = NULL;
}
