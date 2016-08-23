#include "json.h"


/*
static int _parse_primitive(jsmn_parser *parser, const char *js,
		size_t len, jsmntok_t *tokens, size_t num_tokens) {
	jsmntok_t *token;
	int start;

	start = parser->buff_offset;

	for (; parser->buff_offset < len && js[parser->buff_offset] != '\0'; parser->buff_offset++) {
		switch (js[parser->buff_offset]) {
#ifndef JSMN_STRICT
			case ':':
#endif
			case '\t' : case '\r' : case '\n' : case ' ' :
			case ','  : case ']'  : case '}' :
				goto found;
		}
		if (js[parser->buff_offset] < 32 || js[parser->buff_offset] >= 127) {
			parser->buff_offset = start;
			return JSMN_ERROR_INVAL;
		}
	}
#ifdef JSMN_STRICT
	parser->buff_offset = start;
	return JSMN_ERROR_PART;
#endif

found:
	if (tokens == NULL) {
		parser->buff_offset--;
		return 0;
	}
	token = jsmn_alloc_token(parser, tokens, num_tokens);
	if (token == NULL) {
		parser->buff_offset = start;
		return JSMN_ERROR_NOMEM;
	}
	jsmn_fill_token(token, JSMN_PRIMITIVE, start, parser->buff_offset);
#ifdef JSMN_PARENT_LINKS
	token->parent = parser->toksuper;
#endif
	parser->buff_offset--;
	return 0;
}
*/

/**
 * Fills next token with JSON string.
 */
//static int jsmn_parse_string(jsmn_parser *parser, const char *js,
//		size_t len, jsmntok_t *tokens, size_t num_tokens) {
//	jsmntok_t *token;
//
//	int start = parser->buff_offset;
//
//	parser->buff_offset++;
//
//	/* Skip starting quote */
//	for (; parser->buff_offset < len && js[parser->buff_offset] != '\0'; parser->buff_offset++) {
//		char c = js[parser->buff_offset];
//
//		/* Quote: end of string */
//		if (c == '\"') {
//			if (tokens == NULL) {
//				return 0;
//			}
//			token = jsmn_alloc_token(parser, tokens, num_tokens);
//			if (token == NULL) {
//				parser->buff_offset = start;
//				return JSMN_ERROR_NOMEM;
//			}
//			jsmn_fill_token(token, JSMN_STRING, start+1, parser->buff_offset);
//#ifdef JSMN_PARENT_LINKS
//			token->parent = parser->toksuper;
//#endif
//			return 0;
//		}
//
//		/* Backslash: Quoted symbol expected */
//		if (c == '\\' && parser->buff_offset + 1 < len) {
//			int i;
//			parser->buff_offset++;
//			switch (js[parser->buff_offset]) {
//				/* Allowed escaped symbols */
//				case '\"': case '/' : case '\\' : case 'b' :
//				case 'f' : case 'r' : case 'n'  : case 't' :
//					break;
//				/* Allows escaped symbol \uXXXX */
//				case 'u':
//					parser->buff_offset++;
//					for(i = 0; i < 4 && parser->buff_offset < len && js[parser->buff_offset] != '\0'; i++) {
//						/* If it isn't a hex character we have an error */
//						if(!((js[parser->buff_offset] >= 48 && js[parser->buff_offset] <= 57) || /* 0-9 */
//									(js[parser->buff_offset] >= 65 && js[parser->buff_offset] <= 70) || /* A-F */
//									(js[parser->buff_offset] >= 97 && js[parser->buff_offset] <= 102))) { /* a-f */
//							parser->buff_offset = start;
//							return JSMN_ERROR_INVAL;
//						}
//						parser->buff_offset++;
//					}
//					parser->buff_offset--;
//					break;
//				/* Unexpected symbol */
//				default:
//					parser->buff_offset = start;
//					return JSMN_ERROR_INVAL;
//			}
//		}
//	}
//	parser->buff_offset = start;
//	return JSMN_ERROR_PART;
//}
//

enum json_state{
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
    JSON_STATE_SPECIAL,
    JSON_STATE_STR,
    JSON_STATE_STR_ESCAPE,
    JSON_STATE_END_STR,
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
    return ( (state == JSON_STATE_TRUE_ELEM && offs < TRUE_VAL_LEN
              && c == TRUE_VAL[offs])
          || (state == JSON_STATE_NULL_ELEM && offs < NULL_VAL_LEN
              && c == NULL_VAL[offs])
          || (state == JSON_STATE_FALSE_ELEM && offs < FALSE_VAL_LEN
              && c == FALSE_VAL[offs]);
        
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
            break;
        case ',':
            parser->state = JSON_STATE_BEGIN_VAL_OR_KEY;
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
            if (!is_special(parser->key_offset, c, parser->state))
                return JSON_ERROR_PARSING;
            parser->key_offset++;
    }
    return JSON_CONTINUE;
}

//ADD SAVE TO STORAGE
int handle_key_or_str(json_parser *parser, const char c){
    int i;
    json_int_t *prev_int_match = int_matches;
    json_str_t *prev_str_match = str_matches;
    json_int_t *int_match;
    json_str_t *str_match;

    switch (c){
        case '\\':
            parser->state = JSON_STATE_KEY_OR_STR_ESCAPE;
            parser->key_offset++;
            break;
        case '"':
            parser->state = JSON_STATE_KEY_OR_STR_END;
            //mark matches
            for (int_match = parser->int_matches; int_match != JSON_LIST_EMPTY;
                    int_match = int_match->next){
                int_match->match_upto_level++;
            }

            parser->key_offset = 0;
        default:
            //fill the list of matches
            if (parser->should_find_matches){
                for (i=0; i< parser->n_ints; ++i){
                    // Note that keys must contain at least one char
                    // namely '\0'
                    int_match = &int_matches[i];
                    if ((int_match->match_upto_level == parser->level - 1)
                            && (int_match->max_level <= parser->level)
                            && (c == int_match->key_addr[parser->level][0])){
                        prev_int_match->next = int_match;
                        prev_int_match = int_match;
                    }
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
            case JSON_STATE_KEY_OR_STR:
                res = handle_key_or_str(parser, c); break;
            case JSON_STATE_KEY_OR_STR_ESCAPE:
                res = handle_key_or_str_escape(parser, c); break;
            case JSON_STATE_KEY_OR_STR_END:
                res = handle_key_or_str_end(parser, c); break;
            case JSON_STATE_TYPE:
                res = handle_type(parser, c); break;
            case JSON_STATE_INT:
                res = handle_int(parser, c); break;
            case JSON_STATE_SPECIAL:
                res = handle_special(parser, c); break;
            case JSON_STATE_STR:
                res = handle_str(parser, c); break;
            case JSON_STATE_STR_ESCAPE:
                res = handle_str_escape(parser, c); break;
            case JSON_STATE_END_STR:
                res = handle_end_str(parser, c); break;
            case JSON_END_VAL:
                res = handle_end_val(parser, c); break;
            default:
                return JSON_PARSER_FAULT;

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
