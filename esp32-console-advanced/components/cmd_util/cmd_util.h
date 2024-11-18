/**
 * Utility commands
 * @author Seth Teichman
 */

#pragma once
#include "esp_log.h"

/**
 * Log the available Heap Memory
 */
#define UTIL_LOG_FREE(descriptor) ESP_LOGI("cmd_util", "%s Free Heap: %"PRIu32, descriptor, esp_get_free_heap_size());

/**
 * Register utility commands
 */
void register_utils(void);