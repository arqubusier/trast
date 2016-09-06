/* http_get_mbedtls - HTTPS version of the http_get example, using mbed TLS.
 *
 * Retrieves a JSON response from the howsmyssl.com API via HTTPS over TLS v1.2.
 *
 * Validates the server's certificate using the root CA loaded (in PEM format) in cert.c.
 *
 * Adapted from the ssl_client1 example in mbedtls.
 *
 * Original Copyright (C) 2006-2015, ARM Limited, All Rights Reserved, Apache 2.0 License.
 * Additions Copyright (C) 2015 Angus Gratton, Apache 2.0 License.
 */
#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "lwip/api.h"

#include "ssid_config.h"

#include "substr.h"
#include "substr_conf.h"
#include "oauth.h"

/* mbedtls/config.h MUST appear before all other mbedtls headers, or
   you'll get the default config.

   (Although mostly that isn't a big problem, you just might get
   errors at link time if functions don't exist.) */
#include "mbedtls/config.h"

#include "mbedtls/net.h"
#include "mbedtls/debug.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/error.h"
#include "mbedtls/certs.h"
#include "mbedtls/md.h"

/* Add extras/sntp component to makefile for this include to work */
#include <sntp.h>
#include <time.h>

#define SNTP_SERVERS 	"ntp1.sptime.se", "ntp2.sptime.se",\
                        "ntp3.sptime.se", "ntp4.sptime.se"
#define SNTP_UPDATE_INTERVAL 15000 //in ms

#define vTaskDelayMs(ms)	vTaskDelay((ms)/portTICK_RATE_MS)
#define UNUSED_ARG(x)	(void)x

#define MSG_BUFFER_SIZE 2*1024

/* MBEDTLS_DEBUG_C disabled by default to save substantial bloating of
 * firmware, define it in
 * examples/http_get_mbedtls/include/mbedtls/config.h if you'd like
 * debugging output.
 */
#ifdef MBEDTLS_DEBUG_C


/* Increase this value to see more TLS debug details,
   0 prints nothing, 1 will print any errors, 4 will print _everything_
*/
#define DEBUG_LEVEL 0

static void my_debug(void *ctx, int level,
                     const char *file, int line,
                     const char *str)
{
    ((void) level);

    /* Shorten 'file' from the whole file path to just the filename

       This is a bit wasteful because the macros are compiled in with
       the full _FILE_ path in each case, so the firmware is bloated out
       by a few kb. But there's not a lot we can do about it...
    */
    char *file_sep = rindex(file, '/');
    if(file_sep)
        file = file_sep+1;

    printf("%s:%04d: %s", file, line, str);
}
#endif

void setup_home_base(){
    char head[] = "GET&https%3A%2F%2Fapi.twitter.com%2F1.1%2Fstatuses%2Fhome_timeline.json&";
    char count[] = "count%3D";
    int count_val = 3;
    char c_key[] = "%26oauth_consumer_key%3D";
    char c_key_val[] = OAUTH_CONSUMER_KEY;
    char nonce[] =  "%26oauth_nonce%3D";
    char nonce_val[OAUTH_NONCE_LEN+1];
    alpha_num_rand(nonce_val, OAUTH_NONCE_LEN);
    nonce_val[OAUTH_NONCE_LEN] = '\0';
    char sign_met[] = "%26oauth_signature_method%3DHMAC-SHA1";
    char time_stamp[] = "%26oauth_timestamp%3D";
    int  time_stamp_val = 0;
    char token[]  = "%26oauth_token%3D";
    char token_val[]= OAUTH_TOKEN;
    char version[] = "%26oauth_version%3D1.0";
    substr_init(HOME_BASE);
    substr_set_param_str(HOME_BASE, HOME_BASE_HEAD, head);
    substr_set_param_str(HOME_BASE, HOME_BASE_COUNT, count);
    substr_set_param_int(HOME_BASE, HOME_BASE_COUNT_VAL, count_val);
    substr_set_param_str(HOME_BASE, HOME_BASE_OAUTH_CONSUMER_KEY, c_key);
    substr_set_param_str(HOME_BASE, HOME_BASE_OAUTH_CONSUMER_KEY_VAL, c_key_val);
    substr_set_param_str(HOME_BASE, HOME_BASE_OAUTH_NONCE, nonce);
    substr_set_param_str(HOME_BASE, HOME_BASE_OAUTH_NONCE_VAL, nonce_val);
    substr_set_param_str(HOME_BASE, HOME_BASE_OAUTH_TIMESTAMP, time_stamp);
    substr_set_param_int(HOME_BASE, HOME_BASE_OAUTH_TIMESTAMP_VAL, time_stamp_val);
    substr_set_param_str(HOME_BASE, HOME_BASE_OAUTH_TOKEN, token);
    substr_set_param_str(HOME_BASE, HOME_BASE_OAUTH_TOKEN_VAL, token_val);
    substr_set_param_str(HOME_BASE, HOME_BASE_OAUTH_VERSION, version);
    substr_set_param_str(HOME_BASE, HOME_BASE_OAUTH_SIGNATURE_METHOD, sign_met);
}

void setup_home_auth(){
    char head[] = "Authorization: OAuth "; 
    char sign[] = "\", oauth_signature=\"";
    char sign_val[] = "c8CFWImxzs0aVsXnrBSgAIVNZeI%3D";
    char c_key[] = "oauth_consumer_key=\"";
    char c_key_val[] = OAUTH_CONSUMER_KEY;
    char nonce[] =  "\", oauth_nonce=\"";
    char nonce_val[OAUTH_NONCE_LEN+1];
    nonce_val[OAUTH_NONCE_LEN] = '\0';
    char sign_met[] = "\", oauth_signature_method=\"HMAC-SHA1\"";
    char time_stamp[] = ", oauth_timestamp=\"";
    int  time_stamp_val = 0;
    char token[]  = "\", oauth_token=\"";
    char token_val[]= OAUTH_TOKEN;
    char version[] = "\", oauth_version=\"1.0\"\r\n";
    char blank[] = "\r\n";
    substr_init(HOME_AUTH);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_HEAD, head);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_CONSUMER_KEY, c_key);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_CONSUMER_KEY_VAL, c_key_val);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_NONCE, nonce);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_NONCE_VAL, nonce_val);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_TIMESTAMP, time_stamp);
    substr_set_param_int(HOME_AUTH, HOME_AUTH_OAUTH_TIMESTAMP_VAL, time_stamp_val);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_TOKEN, token);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_TOKEN_VAL, token_val);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_VERSION, version);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_SIGNATURE_METHOD, sign_met);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_SIGNATURE, sign);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_SIGNATURE_VAL, sign_val);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_BLANK_LINE, blank);
}

void setup_sign_key(){
    char consumer[] = OAUTH_CONSUMER_SECRET;
    char sep[] = "&";
    char token[] = OAUTH_TOKEN_SECRET;

    substr_init(SIGN_KEY);
    substr_set_param_str(SIGN_KEY, SIGN_KEY_CONSUMER_SECRET, consumer);
    substr_set_param_str(SIGN_KEY, SIGN_KEY_SEP, sep);
    substr_set_param_str(SIGN_KEY, SIGN_KEY_TOKEN_SECRET, token);
}

/* Update nonce, timestamp and oauth signature */
void update_query(){
    char rnd[OAUTH_NONCE_LEN + 1];
    alpha_num_rand(rnd, OAUTH_NONCE_LEN);
    rnd[OAUTH_NONCE_LEN] = '\0';
    substr_set_param_str(HOME_BASE, HOME_BASE_OAUTH_NONCE_VAL, rnd);
    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_NONCE_VAL, rnd);

    time_t time_stamp = time(NULL);
    substr_set_param_long(HOME_BASE, HOME_BASE_OAUTH_TIMESTAMP_VAL,
                          (long) time_stamp);
    substr_set_param_long(HOME_AUTH, HOME_AUTH_OAUTH_TIMESTAMP_VAL,
                          (long) time_stamp);

    size_t base_len = substr_len(HOME_BASE);
    char base[base_len + 1];
    substr_assemble(base, HOME_BASE, base_len + 1);

    size_t sign_key_len = substr_len(SIGN_KEY);
    char sign_key[sign_key_len + 1];
    substr_assemble(sign_key, SIGN_KEY, sign_key_len + 1);

    unsigned char sign[SHA1_LEN];
    const mbedtls_md_info_t *sha_info = mbedtls_md_info_from_type(
                                            MBEDTLS_MD_SHA1);
	mbedtls_md_hmac(sha_info, (unsigned char*) sign_key, 
                    sign_key_len, (unsigned char*) base,
                    base_len, sign);

    printf("BASE: %s\n", base);
    //base64 encode the signature
    const size_t SIGN64_LEN = BASE64_LEN(SHA1_LEN);
    char sign64[SIGN64_LEN + 1];
    base64_encode(sign64, SIGN64_LEN, sign, SHA1_LEN);
    sign64[SIGN64_LEN] = '\0';

    //percent encode the signature
    const size_t SIGN_PERCENT_LEN = percent_encode_size(sign64);
    char sign_percent[SIGN_PERCENT_LEN + 1];
    percent_encode(sign_percent, sign64);
    sign_percent[SIGN_PERCENT_LEN] = '\0';

    substr_set_param_str(HOME_AUTH, HOME_AUTH_OAUTH_SIGNATURE_VAL,
                         sign_percent);
}


void http_get_task(void *pvParameters)
{
    /* Root certs used by twitter, stored in cert.c */
    extern const char* root_certs1;
    extern const char* root_certs2;
    extern const char* root_certs3;
    extern const char* root_certs4;
    extern const char* root_certs5;
    extern const char* root_certs6;

    unsigned int successful_certs_group = 1;
    const size_t N_CERTS_GROUPS = 6;
    const char *root_certs[] = {
        root_certs1, root_certs2, root_certs3,
        root_certs4, root_certs5, root_certs6};
    
    const size_t root_certs_sz[] = {
        strlen(root_certs1)+1, strlen(root_certs2)+1, strlen(root_certs3)+1,
        strlen(root_certs4)+1, strlen(root_certs5)+1, strlen(root_certs6)+1};

    printf("Beginning, free heap = %u\n", xPortGetFreeHeapSize());

    //TODO REMOVE!!!
	UNUSED_ARG(pvParameters);

    int successes = 0, failures = 0, ret;
    printf("STARTING TLS CONNECTION TO TWITTER...\n");

    /* sntp update */
	char *servers[] = {SNTP_SERVERS};
    
	/* Wait until we have joined AP and are assigned an IP */
	while (sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
		vTaskDelayMs(100);
	}

	/* Start SNTP */
	printf("Starting SNTP... ");
	sntp_set_update_delay(SNTP_UPDATE_INTERVAL);
	/* Set GMT zone, daylight savings off */
	const struct timezone tz = {0, 0};
	sntp_initialize(&tz);
	/* Servers must be configured right after initialization */
	sntp_set_servers(servers, sizeof(servers) / sizeof(char*));
	printf("SNTP config complete\n");

    uint32_t flags;
    unsigned char buf[MSG_BUFFER_SIZE];
    const char *pers = "ssl_client1";

    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
    mbedtls_ssl_context ssl;
    mbedtls_x509_crt cacert;
    mbedtls_ssl_config conf;
    mbedtls_net_context server_fd;

    /*
     * 0. Initialize the RNG and the session data
     */
    mbedtls_ssl_init(&ssl);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_ctr_drbg_init(&ctr_drbg);
    printf("\n  . Seeding the random number generator...");

    mbedtls_ssl_config_init(&conf);

    mbedtls_entropy_init(&entropy);
    if((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                    (const unsigned char *) pers,
                                    strlen(pers))) != 0)
    {
        printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
        while(1) {} /* todo: replace with abort() */
    }

    printf(" ok\n");

    /*
     * 0. Initialize certificates
     */
    printf("  . Loading the CA root certificates ...");

    unsigned ca_idx = successful_certs_group;
    unsigned ca_cnt = 0;
    const char *cur_root_certs = root_certs[ca_idx];
    ret = mbedtls_x509_crt_parse(&cacert, (uint8_t*)cur_root_certs,
                                 root_certs_sz[ca_idx]);
    if(ret < 0)
    {
        printf(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
        abort();
    }

    printf(" ok (%d skipped)\n", ret);
    printf("after parsing certificates, free heap = %u\n", 
            xPortGetFreeHeapSize());

    /* Hostname set here should match CN in server certificate */
    if((ret = mbedtls_ssl_set_hostname(&ssl, TWITTER_SERVER)) != 0)
    {
        printf(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
        while(1) {} /* todo: replace with abort() */
    }

    /*
     * 2. Setup stuff
     */
    printf("  . Setting up the SSL/TLS structure...");

    if((ret = mbedtls_ssl_config_defaults(&conf,
                                          MBEDTLS_SSL_IS_CLIENT,
                                          MBEDTLS_SSL_TRANSPORT_STREAM,
                                          MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
    {
        printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
        goto exit;
    }

    printf(" ok\n");

    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
    mbedtls_ssl_conf_max_frag_len(&conf, MBEDTLS_SSL_MAX_FRAG_LEN_2048);
#ifdef MBEDTLS_DEBUG_C
    mbedtls_debug_set_threshold(DEBUG_LEVEL);
    mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);
#endif

    /* Wait until we can resolve the DNS for the server, as an indication
       our network is probably working...
    */
    printf("Waiting for server DNS to resolve... ");
    err_t dns_err;
    ip_addr_t host_ip;
    do {
        vTaskDelay(500 / portTICK_RATE_MS);
        dns_err = netconn_gethostbyname(TWITTER_SERVER, &host_ip);
    } while(dns_err != ERR_OK);
    printf("done.\n");

    printf("before twitter parameters, free heap = %u\n", 
            xPortGetFreeHeapSize());
    /* Set up twitter parameters */
    setup_home_start();
    setup_home_base();
    setup_home_auth();
    setup_sign_key();

    if((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
        goto exit;
    }

    while(1) {
        mbedtls_net_init(&server_fd);
        printf("top of loop, free heap = %u\n", xPortGetFreeHeapSize());
        /*
         * 1. Start the connection
         */
        printf("  . Connecting to %s:%s...", TWITTER_SERVER, TWITTER_PORT);

        if((ret = mbedtls_net_connect(&server_fd, TWITTER_SERVER,
                                      TWITTER_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
        {
            printf(" failed\n  ! mbedtls_net_connect returned %d\n\n", ret);
            goto exit;
        }

        printf(" ok\n");

        mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);



        printf("  . Performing the SSL/TLS handshake...");
        while((ret = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            if (ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED){
                mbedtls_x509_crt_free(&cacert);
                ca_idx++;
                ca_idx %= N_CERTS_GROUPS;
                printf(" failed\n  ! certificate verify failed in handshake"
                       ", trying certificate group %d\n", ca_idx);

                cur_root_certs = root_certs[ca_idx];
                ret = mbedtls_x509_crt_parse(&cacert, (uint8_t*)cur_root_certs,
                                             root_certs_sz[ca_idx]);
                
                if(ret < 0)
                {
                    printf(" failed\n  !  mbedtls_x509_crt_parse "
                           "returned -0x%x\n\n", -ret);
                    abort();
                }

                mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);

            }else if (ret != MBEDTLS_ERR_SSL_WANT_READ && 
                      ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
            }


            goto exit;
        }

        /*
         * The twitter servers return correct size
         */
        size_t buff_sz_result = mbedtls_ssl_get_max_frag_len(&ssl);
        printf("Server decided the size: %d\n", buff_sz_result);

        /*
         * Update twitter parameters
         */
        update_query();

        /*
         * 3. Write the GET request
         */
        printf("  > Write to server:");

        int ids[] = {HOME_START, HOME_AUTH};
        int len = substr_combine((char*) buf, ids, 2, MSG_BUFFER_SIZE); 
        printf("MSGLEN: %d\s\n", len);
        printf("MSG: %s\n", buf);
        if(len == -1)
        {
            printf(" failed\n substr_combine returned %d\n\n", len);
            goto exit;
        }


        while((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0)
        {
            if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
                goto exit;
            }
        }

        len = ret;
        printf(" %d bytes written\n\n%s", len, (char *) buf);

        /*
         * 7. Read the HTTP response
         */
        printf("  < Read from server:");

        do
        {
            len = sizeof(buf) - 1;
            memset(buf, 0, sizeof(buf));
            ret = mbedtls_ssl_read(&ssl, buf, 2048);

            if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
                continue;

            if(ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
                ret = 0;
                break;
            }

            if(ret < 0)
            {
                printf("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret);
                break;
            }

            if(ret == 0)
            {
                printf("\n\nEOF\n\n");
                break;
            }

            len = ret;
            printf(" %d bytes read\n\n%s", len, (char *) buf);
        } while(1);

        mbedtls_ssl_close_notify(&ssl);

    exit:
        mbedtls_ssl_free(&ssl);
        mbedtls_ssl_init(&ssl);
        /* Hostname set here should match CN in server certificate */
        if((ret = mbedtls_ssl_set_hostname(&ssl, TWITTER_SERVER)) != 0)
        {
            printf(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n",
                    ret);
            abort();
        }
        if((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
        {
            printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
            abort();
        }

        mbedtls_net_free(&server_fd);

        if(ret != 0)
        {
            char error_buf[100];
            mbedtls_strerror(ret, error_buf, 100);
            printf("\n\nLast error was: %d - %s\n\n", ret, error_buf);
            failures++;
        } else {
            successes++;
        }

        printf("\n\nsuccesses = %d failures = %d\n", successes, failures);
        for(int countdown = successes ? 10 : 5; countdown >= 0; countdown--) {
            printf("%d... ", countdown);
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
        printf("\nStarting again!\n");
    }
}

void user_init(void)
{
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS
    };

    /* required to call wifi_set_opmode before station_set_config */
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    xTaskCreate(&http_get_task, (signed char *)"get_task", 2048, NULL, 2, NULL);
}
