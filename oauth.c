#include "oauth.h"

const char *oauth_token = "access-token";
const char *consumer_secret = "c-secret";
const char *consumer_key = "c-key";
const char *oauth_token_secret = "acces-secret";

int ouath_init_keys()
{
    return 0;
}

int ouath_update_keys()
{
    return 0;
}

int ouath_free_keys()
{
    return 0;
}

int ouath_request(const char* http_method, const char* url)
{
    return 0;
}

int oauth_base_str(twitter_home_timeline_params params){
    char dst[MAX_SEND_BUF_LEN];
    char *oauth_nonce, oauth_consumer_key, oauth_time_stamp;
    snprintf(dst, MAX_SEND_BUF_LEN, OAUTH_HOME_TIMELINE_BASE,
            params.count,
            params.exclude_replies,
            params.include_entities,
            params.max_id,
            oauth_consumer_key,
            oauth_nonce,
            oauth_time_stamp,
            oauth_token,
            params.since);

    return 0;
}
