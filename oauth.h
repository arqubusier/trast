/*
 * Functions related to Ouath
 *
 * intended to work with only one set of keys at a time.
 *
 */
#include <stdio.h>
#include "twitter.h"

#define MAX_SEND_BUF_LEN 100
#define OAUTH_HOME_TIMELINE_BASE\
    "GET&https%3A%2F%2Fapi.twitter.com%2F1.1%2Fstatuses%2Fhome_timeline.json"\
    "contributor_details\%3D%d"                                              \
    "\%26count\%3D%d"                                                        \
    "\%26exclude_replies\%3D%d"                                              \
    "\%26include_entities\%3D%d"                                             \
    "\%26max_id\%3D%d"                                                       \
    "\%26oauth_consumer_key\%3D%s"                                           \
    "\%26oauth_nonce%3D%s"                                                   \
    "\%26oauth_signature_method\%3DHMAC-SHA1"                                \
    "\%26oauth_timestamp\%3D%s"                                              \
    "\%26oauth_token\%3D%s"                                                  \
    "\%26oauth_version\%3D1.0"                                               \
    "\%26since\%3D%d"                                                        

int oauth_init_keys();
int oauth_update_keys();
int oauth_free_keys();
int oauth_request(twitter_home_timeline_params params);
int oauth_base_str(twitter_home_timeline_params params);
