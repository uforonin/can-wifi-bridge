#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"

static const char *TAG = "bridge_fw";

void app_main(void)
{
    ESP_LOGI(TAG, "bridge_fw started, ESP-IDF %s", esp_get_idf_version());
    while (1) {
        ESP_LOGI(TAG, "alive");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
