#ifndef TWITTER_H
#define TWITTER_H

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

static const twitter_home_timeline_params TWITTER_HOME_TIMELINE_PARAMS_DEFAULTS = 
    {11, 12, 13, 14, 15, 999};

#define VAR_2_STR(varname) #varname

//TODO: only send the parameters != -1 to reduce the string needed to be sent
//TODO: make update function that only updates changed values
int twitter_home_timeline(char* dst, int dst_len,
                          twitter_home_timeline_params params);
#endif /* TWITTER_H */
