#include "can_source.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>

static const char *TAG = "can_emul";

typedef struct {
    uint32_t frame_count;
    TaskHandle_t task;
    QueueHandle_t queue;
    bool running;
} can_emul_priv_t;

static void emul_task(void* arg)
{
    can_emul_priv_t* priv = (can_emul_priv_t*)arg;
    
    while (priv->running) {
        can_frame_t frame;
        frame.id = 0x100 + (priv->frame_count % 4);  /* IDs: 0x100, 0x101, 0x102, 0x103 */
        frame.dlc = 8;
        
        /* Fill with pattern */
        for (int i = 0; i < frame.dlc; i++) {
            frame.data[i] = (uint8_t)(priv->frame_count + i);
        }
        
        frame.timestamp_ms = esp_timer_get_time() / 1000;
        
        xQueueSend(priv->queue, &frame, pdMS_TO_TICKS(10));
        
        priv->frame_count++;
        vTaskDelay(pdMS_TO_TICKS(500));  /* Generate frame every 500ms */
    }
    
    vTaskDelete(NULL);
}

static int emul_start(can_source_t* ctx)
{
    if (!ctx || !ctx->priv) return -1;
    can_emul_priv_t* priv = (can_emul_priv_t*)ctx->priv;
    
    priv->queue = xQueueCreate(16, sizeof(can_frame_t));
    if (!priv->queue) {
        ESP_LOGE(TAG, "Failed to create queue");
        return -1;
    }
    
    priv->running = true;
    priv->frame_count = 0;
    
    if (xTaskCreate(emul_task, "can_emul", 2048, priv, 5, &priv->task) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create task");
        vQueueDelete(priv->queue);
        return -1;
    }
    
    ESP_LOGI(TAG, "Emulator started, generating frames every 500ms");
    return 0;
}

static int emul_stop(can_source_t* ctx)
{
    if (!ctx || !ctx->priv) return -1;
    can_emul_priv_t* priv = (can_emul_priv_t*)ctx->priv;
    
    priv->running = false;
    
    if (priv->task) {
        vTaskDelay(pdMS_TO_TICKS(100));  /* Give task time to exit */
        priv->task = NULL;
    }
    
    if (priv->queue) {
        vQueueDelete(priv->queue);
        priv->queue = NULL;
    }
    
    ESP_LOGI(TAG, "Emulator stopped");
    return 0;
}

static int emul_read(can_source_t* ctx, can_frame_t* frame, uint32_t timeout_ms)
{
    if (!ctx || !ctx->priv || !frame) return -1;
    can_emul_priv_t* priv = (can_emul_priv_t*)ctx->priv;
    
    if (!priv->queue) return -1;
    
    if (xQueueReceive(priv->queue, frame, pdMS_TO_TICKS(timeout_ms)) == pdTRUE) {
        return 1;
    }
    return 0;  /* Timeout, no frame */
}

static int emul_write(can_source_t* ctx, const can_frame_t* frame)
{
    /* Emulator doesn't support write, just log */
    if (!frame) return -1;
    ESP_LOGI(TAG, "Write (ignored): ID=0x%03lX, DLC=%u", (unsigned long)frame->id, frame->dlc);
    return 0;
}

can_source_t* can_source_create_emul(void)
{
    can_source_t* src = calloc(1, sizeof(can_source_t));
    if (!src) return NULL;
    
    can_emul_priv_t* priv = calloc(1, sizeof(can_emul_priv_t));
    if (!priv) {
        free(src);
        return NULL;
    }
    
    src->priv = priv;
    src->start = emul_start;
    src->stop = emul_stop;
    src->read = emul_read;
    src->write = emul_write;
    
    ESP_LOGI(TAG, "Emulator created");
    return src;
}
