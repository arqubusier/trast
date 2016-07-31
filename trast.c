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

//#include <time.h>

#define WEB_SERVER "www.howsmyssl.com"
#define WEB_PORT "443"
#define WEB_URL "https://www.howsmyssl.com/a/check"

#define GET_REQUEST "GET "WEB_URL" HTTP/1.1\nHost: "WEB_SERVER"\n\n"

/* Root cert for howsmyssl.com, stored in cert.c */
extern const char *server_root_cert;

/* MBEDTLS_DEBUG_C disabled by default to save substantial bloating of
 * firmware, define it in
 * examples/http_get_mbedtls/include/mbedtls/config.h if you'd like
 * debugging output.
 */
#ifdef MBEDTLS_DEBUG_C


/* Increase this value to see more TLS debug details,
   0 prints nothing, 1 will print any errors, 4 will print _everything_
*/
#define DEBUG_LEVEL 4

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
    char head[] = "GET&https%%3A%%2F%%2Fapi.twitter.com%%2F1.1%%2Fstatuses%%2Fhome_timeline.json&"; 
    char c_key[] = "oauth_consumer_key%%3D";
    char c_key_val[] = OAUTH_CONSUMER_KEY;
    char nonce[] =  "%%26oauth_nonce%%3D";
    char nonce_val[OAUTH_NONCE_LEN+1];
    nonce_val[OAUTH_NONCE_LEN] = '\0';
    char sign_met[] = "%%26oauth_signature_method%%3DHMAC-SHA1";
    char time_stamp[] = "%%26oauth_timestamp%%3D";
    //int  time_stamp_val = (int) time(NULL);
    int  time_stamp_val = 0;
    char token[]  = "%%26oauth_token%%3D";
    char token_val[]= OAUTH_TOKEN;
    char version[] = "%%26oauth_version%%3D1.0";
    substr_init(HOME_BASE);
    substr_set_param_str(HOME_BASE, HOME_BASE_HEAD, head);
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
    char sign_met[] = "\",oauth_signature_method=\"3DHMAC-SHA1\",";
    char time_stamp[] = "oauth_timestamp=\"";
    //int  time_stamp_val =   (int) time(NULL);
    int  time_stamp_val = 0;
    char token[]  = "\", oauth_token=\"";
    char token_val[]= OAUTH_TOKEN;
    char version[] = "\"oauth_version=\"1.0\"";
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

}

void setup_home_start(){
    char head[] = "https://api.twitter.com/1.1/statuses/home_timeline.json";
    char count[] = "?count=";
    int count_val = 5;
    substr_init(HOME_START);
    substr_set_param_str(HOME_START, HOME_START_HEAD, head);
    substr_set_param_str(HOME_START, HOME_START_COUNT, count);
    substr_set_param_int(HOME_START, HOME_START_COUNT_VAL, count_val);

}


void http_get_task(void *pvParameters)
{
    int successes = 0, failures = 0, ret;
    printf("HTTP get task starting...\n");

    uint32_t flags;
    unsigned char buf[1024];
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
    printf("  . Loading the CA root certificate ...");

    ret = mbedtls_x509_crt_parse(&cacert, (uint8_t*)server_root_cert, strlen(server_root_cert)+1);
    if(ret < 0)
    {
        printf(" failed\n  !  mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
        while(1) {} /* todo: replace with abort() */
    }

    printf(" ok (%d skipped)\n", ret);

    /* Hostname set here should match CN in server certificate */
    if((ret = mbedtls_ssl_set_hostname(&ssl, WEB_SERVER)) != 0)
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

    /* OPTIONAL is not optimal for security, in this example it will print
       a warning if CA verification fails but it will continue to connect.
    */
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_OPTIONAL);
    mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
#ifdef MBEDTLS_DEBUG_C
    mbedtls_debug_set_threshold(DEBUG_LEVEL);
    mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);
#endif

    if((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0)
    {
        printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
        goto exit;
    }

    /* Wait until we can resolve the DNS for the server, as an indication
       our network is probably working...
    */
    printf("Waiting for server DNS to resolve... ");
    err_t dns_err;
    ip_addr_t host_ip;
    do {
        vTaskDelay(500 / portTICK_RATE_MS);
        dns_err = netconn_gethostbyname(WEB_SERVER, &host_ip);
    } while(dns_err != ERR_OK);
    printf("done.\n");

    while(1) {
        mbedtls_net_init(&server_fd);
        printf("top of loop, free heap = %u\n", xPortGetFreeHeapSize());
        /*
         * 1. Start the connection
         */
        printf("  . Connecting to %s:%s...", WEB_SERVER, WEB_PORT);

        if((ret = mbedtls_net_connect(&server_fd, WEB_SERVER,
                                      WEB_PORT, MBEDTLS_NET_PROTO_TCP)) != 0)
        {
            printf(" failed\n  ! mbedtls_net_connect returned %d\n\n", ret);
            goto exit;
        }

        printf(" ok\n");

        mbedtls_ssl_set_bio(&ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

        /*
         * 4. Handshake
         */
        printf("  . Performing the SSL/TLS handshake...");

        while((ret = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            if(ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
                goto exit;
            }
        }

        printf(" ok\n");

        /*
         * 5. Verify the server certificate
         */
        printf("  . Verifying peer X.509 certificate...");

        /* In real life, we probably want to bail out when ret != 0 */
        if((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
        {
            char vrfy_buf[512];

            printf(" failed\n");

            mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "  ! ", flags);

            printf("%s\n", vrfy_buf);
        }
        else
            printf(" ok\n");

        /*
         * 3. Write the GET request
         */
        printf("  > Write to server:");

        int len = sprintf((char *) buf, GET_REQUEST);

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
            ret = mbedtls_ssl_read(&ssl, buf, len);

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
        mbedtls_ssl_session_reset(&ssl);
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

    setup_home_start();
    setup_home_base();
    setup_home_auth();


    size_t start_len = substr_size(HOME_START);
    size_t auth_len = substr_size(HOME_AUTH);
    size_t base_len = substr_size(HOME_BASE);

    char start[start_len];
    char auth[auth_len];
    char base[base_len];

    char rnd[OAUTH_NONCE_LEN + 1];
    rnd[OAUTH_NONCE_LEN] = '\0';
    alpha_num_rand(rnd, OAUTH_NONCE_LEN);
    substr_set_param_str(HOME_BASE, HOME_BASE_OAUTH_NONCE_VAL, rnd);

    substr_assemble(start, HOME_START, start_len);
    substr_assemble(auth, HOME_AUTH, auth_len);
    substr_assemble(base, HOME_BASE, base_len);

    printf(start);
    printf("\n");
    printf(auth);
    printf("\n");
    printf(base);
    printf("\n");

    size_t sign_key_len = OAUTH_CONSUMER_SECRET_LEN
                          + OAUTH_TOKEN_SECRET_LEN + 1;
    unsigned char sign_key[OAUTH_CONSUMER_SECRET_LEN + 1 + OAUTH_TOKEN_SECRET_LEN + 1] = 
            OAUTH_CONSUMER_SECRET "&" OAUTH_TOKEN_SECRET;
            sign_key[OAUTH_CONSUMER_SECRET_LEN + 1 + OAUTH_TOKEN_SECRET_LEN] = '\0';
    unsigned char sign[SHA1_LEN];

    const mbedtls_md_info_t *sha_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA1);
	mbedtls_md_hmac(sha_info, sign_key, sign_key_len, (unsigned char*) base,
                    base_len, sign);

    const size_t SIGN64_LEN = BASE64_LEN(SHA1_LEN);
    char sign64[SIGN64_LEN + 1];
    printf("sign %02X %02X %02X\n", sign[0], sign[1], sign[2]);
    base64_encode(sign64, SIGN64_LEN, sign, SHA1_LEN);
    sign64[SIGN64_LEN] = '\0';

    printf("sign %02X %02X %02X\n", sign[0], sign[1], sign[2]);
    print_hex(sign, SHA1_LEN);
    printf("\n");
    printf(sign_key);
    printf("\n");
    printf(sign64);
    printf("\n");
    

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };

    /* required to call wifi_set_opmode before station_set_config */
    /*
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

    xTaskCreate(&http_get_task, (signed char *)"get_task", 2048, NULL, 2, NULL);
    */
}
