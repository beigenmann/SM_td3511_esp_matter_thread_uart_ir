#include "matter_meter.hpp"
#include "esp_log.h"

static const char *TAG = "matter_meter";

esp_err_t matter_meter_init(void)
{
    /*
     * TODO: integrate against the exact ESP-Matter release selected:
     * - Matter 1.4 Electrical Power Measurement cluster (0x0090)
     * - Electrical Energy Measurement cluster (0x0091)
     * - Thread commissioning/network setup
     * - Polyphase feature / supported attributes and reporting
     *
     * Do not guess generated API names across SDK versions.
     */
    ESP_LOGE(TAG, "Matter adapter not yet bound to a specific ESP-Matter checkout");
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t matter_meter_publish(const meter_snapshot_t &sample)
{
    (void)sample;
    // TODO: map units/scaling/nullable values to cluster attributes.
    return ESP_ERR_NOT_SUPPORTED;
}
