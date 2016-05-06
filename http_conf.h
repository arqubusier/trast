/*
 * Defines for substrings used in http messages
 *
 * TODO: add explanation
 */
enum twitter_statuses_home_timeline_start{
    home_start_head;
    home_start_count;
    home_start_count_val;
    home_start_since_id;
    home_start_since_id_val;
    home_start_max_id;
    home_start_max_id_val;
    home_start_trim_user;
    home_start_trim_user_val;
    home_start_exclude_replies;
    home_start_exclude_replies_val;
    home_start_contributor_details;
    home_start_contributor_details_val;
    home_start_include_entities;
    home_start_include_entities_val;
    home_start;
}

enum oauth_home_timeline_base_str{
    home_base_head;
    home_base_count;
    home_base_count_val;
    home_base_contributor_details;
    home_base_contributor_details_val;
    home_base_exclude_replies;
    home_base_exclude_replies_val;
    home_base_include_entities;
    home_base_include_entities_val;
    home_base_max_id;
    home_base_max_id_val;
    home_base_oauth_consumer_key;
    home_base_oauth_consumer_key_val;
    home_base_oauth_nonce;
    home_base_oauth_nonce_val;
    home_base_oauth_signature_method;
    home_base_oauth_timestamp;
    home_base_oauth_timestamp_val;
    home_base_oauth_token;
    home_base_oauth_token_val;
    home_base_oauth_version;
    home_base_since_id;
    home_base_since_id_val;
    home_base_trim_user;
    home_base_trim_user_val;
    home_base;
}

enum oauth_home_timeline_auth_header{
    home_auth_oauth_nonce;
    home_auth_oauth_nonce_val;
    home_auth_oauth_signature;
    home_auth_oauth_signature_val;
    home_auth_oauth_signature_method;
    home_auth_oauth_timestamp;
    home_auth_oauth_timestamp_val;
    home_auth_oauth_token;
    home_auth_oauth_token_val;
    home_auth_oauth_version;
    home_auth;
}

int substr_n_params[] = {home_start, home_base, home_auth};

#define SUBSTR_N_PARAMS_LEN  sizeof(substr_n_params)/sizeof(int)
#define SUBSTR_N_PARAMS(substr_id) substr_n_params[substr_id]
#define PARAM_DEFAULT_LEN 5
