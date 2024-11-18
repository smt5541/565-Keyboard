/**
 * Utility commands
 * @author Seth Teichman
 */

#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "esp_console.h"

/** Arguments used by 'join' function */
static struct {
  struct arg_end *end;
} util_free_args;

static int cmd_util_free(int argc, char **argv) {
  printf("%"PRIu32"\n", esp_get_free_heap_size());
  return 0;
}

void register_utils() {
    util_free_args.end = arg_end(0);

    const esp_console_cmd_t util_free_cmd = {
        .command = "util_free",
        .help = "Get free Heap Memory",
        .hint = NULL,
        .func = &cmd_util_free,
        .argtable = &util_free_args
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&util_free_cmd) );
}