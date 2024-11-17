/**
 * HTTP Wrappers, including REST over cJSON
 * @author Seth Teichman
 */

#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#if CONFIG_MBEDTLS_CERTIFICATE_BUNDLE
#include "esp_crt_bundle.h"
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include "esp_http_client.h"

#define MAX_HTTP_RECV_BUFFER 512
#define MAX_HTTP_OUTPUT_BUFFER 2048
static const char *TAG = "http";
#include "http.h"
#include "cJSON.h"

#define CONFIG_EXAMPLE_HTTP_ENDPOINT "192.168.0.127"

static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            // Clean the buffer in case of a new request
            if (output_len == 0 && evt->user_data) {
                // we are just starting to copy the output data into the use
                memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
            }
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                int copy_len = 0;
                if (evt->user_data) {
                    // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
                    copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                    if (copy_len) {
                        memcpy(evt->user_data + output_len, evt->data, copy_len);
                    }
                } else {
                    int content_len = esp_http_client_get_content_length(evt->client);
                    if (output_buffer == NULL) {
                        // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
                        output_buffer = (char *) calloc(content_len + 1, sizeof(char));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    copy_len = MIN(evt->data_len, (content_len - output_len));
                    if (copy_len) {
                        memcpy(output_buffer + output_len, evt->data, copy_len);
                    }
                }
                output_len += copy_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
                ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
            }
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            esp_http_client_set_header(evt->client, "From", "user@example.com");
            esp_http_client_set_header(evt->client, "Accept", "text/html");
            esp_http_client_set_redirection(evt->client);
            break;
    }
    return ESP_OK;
}

void http_get_method_name(esp_http_client_method_t method, char **methodName) {
  switch (method) {
    case HTTP_METHOD_GET:
      *methodName = "GET";
      break;
    case HTTP_METHOD_POST:
      *methodName = "POST";
      break;
    case HTTP_METHOD_PUT:
      *methodName = "PUT";
      break;
    case HTTP_METHOD_PATCH:
      *methodName = "PATCH";
      break;
    case HTTP_METHOD_DELETE:
      *methodName = "DELETE";
      break;
    default:
      ESP_LOGW(TAG, "http_get_method_name received unexpected method: %d - returning UNKNOWN", method);
      *methodName = "UNKNOWN";
  }
}

esp_http_client_method_t http_get_method_enum(const char *methodName) {
  if (strcmp(methodName, "GET") == 0) {
    return HTTP_METHOD_GET;
  } else if (strcmp(methodName, "POST") == 0) {
    return HTTP_METHOD_POST;
  } else if (strcmp(methodName, "PUT") == 0) {
    return HTTP_METHOD_PUT;
  } else if (strcmp(methodName, "PATCH") == 0) {
    return HTTP_METHOD_PATCH;
  } else if (strcmp(methodName, "DELETE") == 0) {
    return HTTP_METHOD_DELETE;
  } else {
    ESP_LOGW(TAG, "http_get_method_enum received unexpected methodName: %s - returning GET", methodName);
    return HTTP_METHOD_GET;
  }
}

bool http_request(esp_http_client_config_t *config, http_response_t *response) {
  bool status = true;
  // Declare local_response_buffer with size (MAX_HTTP_OUTPUT_BUFFER + 1) to prevent out of bound access when
  // it is used by functions like strlen(). The buffer should only be used upto size MAX_HTTP_OUTPUT_BUFFER
  char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
  config->event_handler = _http_event_handler;
  config->user_data = local_response_buffer;
  esp_http_client_handle_t client = esp_http_client_init((const esp_http_client_config_t*) config);

  esp_err_t err = esp_http_client_perform(client);
  char *methodName;
  http_get_method_name(config->method, &methodName);
  if (err == ESP_OK) {
    response->attrs.statusCode = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "HTTP %s Status = %d, content_length = %"PRId64,
      methodName,
      response->attrs.statusCode,
      esp_http_client_get_content_length(client));
  } else {
      ESP_LOGE(TAG, "HTTP %s request failed: %s", methodName, esp_err_to_name(err));
      goto l_err;
  }
  ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));

  int responseLength = strlen(local_response_buffer)+1;
  response->body = malloc(responseLength);
  memcpy(response->body, &local_response_buffer, responseLength);
  ESP_LOGI(TAG, "HTTP Response Body: %s", response->body);
l_end:
  esp_http_client_cleanup(client);
  return status;
l_err:
  status = false;
  goto l_end;
}

bool http_request_body(esp_http_client_config_t *config, http_request_data_t *requestData, http_response_t *response) {
  bool status = true;
  char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER + 1] = {0};
  config->event_handler = _http_event_handler;
  config->user_data = local_response_buffer;
  esp_http_client_handle_t client = esp_http_client_init((const esp_http_client_config_t*) config);

  esp_http_client_set_header(client, "Content-Type", requestData->contentType);
  esp_http_client_set_post_field(client, requestData->body, strlen(requestData->body));

  esp_err_t err = esp_http_client_perform(client);
  char *methodName;
  http_get_method_name(config->method, &methodName);
  if (err == ESP_OK) {
    response->attrs.statusCode = esp_http_client_get_status_code(client);
    ESP_LOGI(TAG, "HTTP %s Status = %d, content_length = %"PRId64,
      methodName,
      response->attrs.statusCode,
      esp_http_client_get_content_length(client));
  } else {
      ESP_LOGE(TAG, "HTTP %s request failed: %s", methodName, esp_err_to_name(err));
      goto l_err;
  }
  ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));

  int responseLength = strlen(local_response_buffer)+1;
  response->body = malloc(responseLength);
  memcpy(response->body, &local_response_buffer, responseLength);
  ESP_LOGI(TAG, "HTTP Response Body: %s", response->body);
  l_end:
    esp_http_client_cleanup(client);
    return status;
  l_err:
    status = false;
    goto l_end;
}

bool http_rest_request(esp_http_client_config_t *config, http_rest_response_t *response) {
  bool status = true;
  http_response_t rawResponse = {0};
  bool baseSuccess = http_request(config, &rawResponse);
  if (!baseSuccess) goto l_err;

  response->attrs.statusCode = rawResponse.attrs.statusCode;
  response->body = cJSON_Parse(rawResponse.body);
  if (response->body == NULL) goto l_err;
l_end:
  if (rawResponse.body != NULL) free(rawResponse.body);
  return status;
l_err:
  status = false;
  goto l_end;
}

bool http_rest_request_body(esp_http_client_config_t *config, http_rest_data_t *restData, http_rest_response_t *response) {
  bool status = true;
  http_request_data_t requestData = {0};
  requestData.body = cJSON_PrintUnformatted(restData->body);
  requestData.contentType = "application/json";
  if (requestData.body == NULL) goto l_err;

  http_response_t rawResponse = {0};
  bool baseSuccess = http_request_body(config, &requestData, &rawResponse);
  if (!baseSuccess) goto l_err;

  response->attrs.statusCode = rawResponse.attrs.statusCode;
  response->body = cJSON_Parse(rawResponse.body);
  if (response->body == NULL) goto l_err;
l_end:
  if (requestData.body != NULL) free(requestData.body);
  if (rawResponse.body != NULL) free(rawResponse.body);
  return status;
l_err:
  status = false;
  goto l_end;
}
