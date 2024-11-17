/**
 * HTTP Wrappers, including REST over cJSON
 * @author Seth Teichman
 */

#pragma once
#include <stdbool.h>
#include "esp_http_client.h"
#include "cJSON.h"

/**
 * Contains HTTP request body information
 */
typedef struct http_request_data_t {
  /** The body text, must not be NULL */
  char *body;

  /** The Content-Type header to send with the body */
  char *contentType;
} http_request_data_t;

/** Contains HTTP REST request body information */
typedef struct http_rest_data_t {
  /** The JSON body to send */
  cJSON *body;
} http_rest_data_t;

/** Common HTTP response information */
typedef struct http_response_common_t {
  /** The HTTP Status Code returned by the server */
  int statusCode;
} http_response_common_t;

/** Raw HTTP response information */
typedef struct http_response_t {
  /** Common HTTP response information */
  http_response_common_t attrs;

  /** The response body */
  char *body;
} http_response_t;

/** HTTP REST response information */
typedef struct http_rest_response_t {
  /** Common HTTP response information */
  http_response_common_t attrs;

  /** The response body, may be NULL */
  cJSON *body;
} http_rest_response_t;

/**
 * Translate esp_http_client_method_t into strings
 * @param method The member of esp_http_client_method_t to translate
 * @param methodName Pointer to the string memory the result should be written to
 */
void http_get_method_name(esp_http_client_method_t method, char **methodName);

/**
 * Translate strings into esp_http_client_method_t
 * @param methodName The string to translate
 * @return The resulting esp_http_client_method_t
 */
esp_http_client_method_t http_get_method_enum(const char *methodName);

/**
 * Perform a plain HTTP Request
 * @param config The request configuration
 * @param response Pointer to the initialized response object
 */
bool http_request(esp_http_client_config_t *config, http_response_t *response);

/**
 * Perform a plain HTTP Request with a body
 * @param config The request configuration
 * @param requestData The request body information
 * @param response Pointer to the initialized response object
 */
bool http_request_body(esp_http_client_config_t *config, http_request_data_t *requestData, http_response_t *response);

/**
 * Perform a HTTP REST Request
 * @param config The request configuration
 * @param response Pointer to the initialized response object
 */
bool http_rest_request(esp_http_client_config_t *config, http_rest_response_t *response);

/**
 * Perform a HTTP REST Request with a body
 * @param config The request configuration
 * @param restData The request body information
 * @param response Pointer to the initialized response object
 */
bool http_rest_request_body(esp_http_client_config_t *config, http_rest_data_t *restData, http_rest_response_t *response);