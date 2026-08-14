idf_component_register(SRCS "main.c" "can_emul.c" "can_twai.c" "bridge_proto.c" "tcp_server.c"
        INCLUDE_DIRS ""
        REQUIRES driver esp_wifi nvs_flash led_strip esp_timer lwip)