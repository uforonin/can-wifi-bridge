#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "led_strip.h"
#include "can_source.h"

static const char *TAG = "bridge_fw";

/* Wi-Fi AP credentials */
#define WIFI_AP_SSID      "Bridge_AP"
#define WIFI_AP_PASS      "bridge123"
#define WIFI_AP_CHANNEL   1
#define WIFI_AP_MAX_CONN  4

/* RGB LED on GPIO48 (ESP32-S3-Zero) */
#define LED_GPIO          48
#define LED_STRIP_LED_NUM 1

static led_strip_handle_t led_strip;
static can_source_t* can_src = NULL;

/* Wi-Fi event handler */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station connected, AID=%d", event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station disconnected, AID=%d", event->aid);
    }
}

static void led_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = LED_STRIP_LED_NUM,
        .led_model = LED_MODEL_WS2812,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,  /* 10 MHz */
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
    ESP_LOGI(TAG, "LED strip initialized on GPIO%d", LED_GPIO);
}

static void wifi_ap_init(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = WIFI_AP_SSID,
            .ssid_len = strlen(WIFI_AP_SSID),
            .channel = WIFI_AP_CHANNEL,
            .password = WIFI_AP_PASS,
            .max_connection = WIFI_AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = { .required = true },
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi AP started: SSID=%s, password=%s, channel=%d",
             WIFI_AP_SSID, WIFI_AP_PASS, WIFI_AP_CHANNEL);
}

static void can_source_task(void* arg)
{
    can_frame_t frame;
    
    while (1) {
        int res = can_src->read(can_src, &frame, 100);
        if (res > 0) {
            ESP_LOGI(TAG, "CAN frame: ID=0x%03lX, DLC=%u, data=[%02X %02X %02X %02X %02X %02X %02X %02X]",
                     (unsigned long)frame.id, frame.dlc,
                     frame.data[0], frame.data[1], frame.data[2], frame.data[3],
                     frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "bridge_fw started, ESP-IDF %s", esp_get_idf_version());

    /* Initialize NVS (required for Wi-Fi) */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    led_init();
    wifi_ap_init();

    /* Initialize CAN source (emulator for now) */
    can_src = can_source_create_emul();
    if (!can_src) {
        ESP_LOGE(TAG, "Failed to create CAN source");
        return;
    }
    
    if (can_src->start(can_src) != 0) {
        ESP_LOGE(TAG, "Failed to start CAN source");
        return;
    }
    
    /* Create task to read CAN frames */
    xTaskCreate(can_source_task, "can_read", 4096, NULL, 5, NULL);

    /* Main loop: blink LED blue, 1s period */
    while (1) {
        led_strip_set_pixel(led_strip, 0, 0, 0, 16);  /* Blue */
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(500));

        led_strip_clear(led_strip);
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
