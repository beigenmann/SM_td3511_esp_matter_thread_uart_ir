#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "ir_reader.hpp"
#include "matter_meter.hpp"

static const char *TAG = "SM_td3511";

extern "C" void app_main(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(ir_reader_init());
    ESP_ERROR_CHECK(matter_meter_init());

    while (true) {
        meter_snapshot_t sample{};
        if (ir_reader_get_snapshot(&sample) == ESP_OK) {
            esp_err_t update_err = matter_meter_publish(sample);
            if (update_err != ESP_OK) {
                ESP_LOGW(TAG, "Matter update failed: %s", esp_err_to_name(update_err));
            }
        } else {
            ESP_LOGW(TAG, "No valid meter snapshot yet");
        }
        vTaskDelay(pdMS_TO_TICKS(CONFIG_SM_TD3511_SAMPLE_PERIOD_MS));
    }
}
