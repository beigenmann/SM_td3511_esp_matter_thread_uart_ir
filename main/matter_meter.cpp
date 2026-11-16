#include "matter_meter.hpp"
#include "esp_matter.h"
#include "esp_matter_cluster.h"
#include "esp_log.h"
#include "app/clusters/electrical-power-measurement-server/electrical-power-measurement-server.h"

using namespace esp_matter;
using namespace esp_matter::cluster;

static constexpr char tag[] = "matter_meter";
static uint16_t s_endpoint_id = 0;

esp_err_t matter_meter_init()
{
    ESP_LOGI(tag, "Initializing Electrical Power Measurement cluster");

    auto node_ptr = node::get();
    if (!node_ptr) {
        ESP_LOGE(tag, "Matter node not initialized yet");
        return ESP_ERR_INVALID_STATE;
    }

    auto endpoint_ptr = endpoint::electrical_sensor::create(node_ptr, nullptr, ENDPOINT_FLAG_NONE, nullptr);
    if (!endpoint_ptr) {
        ESP_LOGE(tag, "Failed to create electrical sensor endpoint");
        return ESP_ERR_NO_MEM;
    }

    s_endpoint_id = endpoint::get_id(endpoint_ptr);

    auto epm_cluster = cluster::get(endpoint_ptr, chip::app::Clusters::ElectricalPowerMeasurement::Id);
    if (epm_cluster) {
        electrical_power_measurement::feature::alternating_current::add(epm_cluster);

        int64_t initial_val = 0;
        electrical_power_measurement::attribute::create_voltage(epm_cluster, initial_val);
        electrical_power_measurement::attribute::create_active_current(epm_cluster, initial_val);
        electrical_power_measurement::attribute::create_active_power(epm_cluster, initial_val);
    }

    ESP_LOGI(tag, "Matter Electrical Power Measurement cluster initialized successfully");
    return ESP_OK;
}

esp_err_t matter_meter_publish(const meter_snapshot_t &sample)
{
    esp_matter_attr_val_t val = {};

    if (sample.l1.voltage_valid) {
        val.type = ESP_MATTER_VAL_TYPE_NULLABLE_INT64;
        val.val.i64 = static_cast<int64_t>(sample.l1.voltage_v * 1000.0f);
        attribute::update(s_endpoint_id, chip::app::Clusters::ElectricalPowerMeasurement::Id, chip::app::Clusters::ElectricalPowerMeasurement::Attributes::Voltage::Id, &val);
    }

    if (sample.l1.current_valid) {
        val.type = ESP_MATTER_VAL_TYPE_NULLABLE_INT64;
        val.val.i64 = static_cast<int64_t>(sample.l1.current_a * 1000.0f);
        attribute::update(s_endpoint_id, chip::app::Clusters::ElectricalPowerMeasurement::Id, chip::app::Clusters::ElectricalPowerMeasurement::Attributes::ActiveCurrent::Id, &val);
    }

    if (sample.l1.power_valid) {
        val.type = ESP_MATTER_VAL_TYPE_NULLABLE_INT64;
        val.val.i64 = static_cast<int64_t>(sample.l1.active_power_w * 1000.0f);
        attribute::update(s_endpoint_id, chip::app::Clusters::ElectricalPowerMeasurement::Id, chip::app::Clusters::ElectricalPowerMeasurement::Attributes::ActivePower::Id, &val);
    }

    return ESP_OK;
}