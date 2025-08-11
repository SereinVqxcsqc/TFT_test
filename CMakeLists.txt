idf_component_register(SRCS "wifi_manager.c" "http_server.c" "main.c" "st7735s.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_http_server nvs_flash json spiffs esp_led_strip)