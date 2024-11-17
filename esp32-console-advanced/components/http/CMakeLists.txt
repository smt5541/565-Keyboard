set(requires "")
list(APPEND requires console esp_wifi esp_event esp-tls esp_http_client nvs_flash cjson cmd_util)
idf_component_register(SRCS "http.c" "cmd_http.c"
                    INCLUDE_DIRS .
                    REQUIRES ${requires})
