/**
 * HTTP Commands
 * @author Seth Teichman
 */

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "http.h"

static const char *TAG = "cmd_http";

/** Arguments used by 'http_rest_request' command */
static struct {
  struct arg_str *method;
  struct arg_str *url;
  struct arg_str *body;
  struct arg_end *end;
} http_rest_request_args;

static int cmd_http_rest_request(int argc, char **argv) {
  int status = 0;
  http_rest_data_t requestData = {0};
  char *responseBody = NULL;
  int nerrors = arg_parse(argc, argv, (void **) &http_rest_request_args);
  if (nerrors != 0) {
      arg_print_errors(stderr, http_rest_request_args.end, argv[0]);
      goto l_end;
  }

  esp_http_client_config_t requestCfg = {
    .host = "192.168.0.127",
    .port = 5000,
    .method = http_get_method_enum(http_rest_request_args.method->sval[0]),
    .url = http_rest_request_args.url->sval[0]
  };
  http_rest_response_t responseData = {0};

  bool rest_success = false;
  if (http_rest_request_args.body->count == 0) {
    rest_success = http_rest_request(&requestCfg, &responseData);
  } else {
    requestData.body = cJSON_Parse(http_rest_request_args.body->sval[0]);
    if (requestData.body == NULL) {
      ESP_LOGW(TAG, "Failed to parse JSON from parameter [body]");
      goto l_err;
    }
    rest_success = http_rest_request_body(&requestCfg, &requestData, &responseData);
  }

  if (responseData.body != NULL) {
    responseBody = cJSON_Print(responseData.body);
    cJSON_Delete(responseData.body);
  }
  if (rest_success) {
    ESP_LOGI(TAG, "Response Code: %d, Body: %s", responseData.attrs.statusCode, responseBody);
    goto l_end;
  } else {
    ESP_LOGI(TAG, "Response Code: %d", responseData.attrs.statusCode);
    goto l_err;
  }
l_end:
  if (requestData.body != NULL) cJSON_Delete(requestData.body);
  if (responseBody != NULL) free(responseBody);
  return status;
l_err:
  goto l_end;
}

void register_http(void) {
    http_rest_request_args.method = arg_str1(NULL, NULL, "<method>", "The HTTP Method to send in the Request");
    http_rest_request_args.url = arg_str1(NULL, NULL, "<url>", "The URL to send the Request to");
    http_rest_request_args.body = arg_str0(NULL, NULL, "[body]", "The Request Body JSON to send");
    http_rest_request_args.end = arg_end(2);

    const esp_console_cmd_t http_rest_request_cmd = {
        .command = "http_rest_request",
        .help = "Send an HTTP REST Request",
        .hint = NULL,
        .func = &cmd_http_rest_request,
        .argtable = &http_rest_request_args
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&http_rest_request_cmd) );
}
