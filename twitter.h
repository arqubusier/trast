#define TWITTER_URL_HOME_TIMELINE \
    "https://api.twitter.com/1.1/statuses/user_timeline.json"
#define TWITTER_URL_HOME_TIMELINE_PARAMS\
    "/1.1/statuses/user_timeline.json?" \
    "contributor_details=%d"            \
    "&count=%d"                         \
    "&exclude_replies=%d"               \
    "&include_entities=%d"              \
    "&max_id=%d"                        \
    "&since=%d"                         

typedef struct twitter_home_timeline_params{
    int count, since, max_id, exclude_replies, contributor_details,
        include_entities;
}twitter_home_timeline_params;

const twitter_home_timeline_params TWITTER_HOME_TIMELINE_PARAMS_DEFAULTS = 
    {0, 0, 0, 0, 0, 0};

#define VAR_2_STR(varname) #varname

//TODO: only send the parameters != -1 to reduce the string needed to be sent
int twitter_home_timeline(char * dst, int dst_len,
                          twitter_home_timeline_params params){
   return snprintf(dst, dst_len, TWITTER_URL_HOME_TIMELINE_PARAMS,
                   params.count,
                   params.contributor_details,
                   params.exclude_replies,
                   params.include_entities,
                   params.max_id,
                   params.since);
}
