#include "matter_meter.hpp"
#include "esp_matter.h"
#include "esp_matter_cluster.h"
#include "esp_log.h"
#include "app/clusters/electrical-power-measurement-server/electrical-power-measurement-server.h"

using namespace esp_matter;
using namespace esp_matter::cluster;

static constexpr char tag[] = "matter_meter";
static uint16_t s_endpoint_id = 0;


// Basic Matter event callback (required for node::create)
static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    // Handle commissioning or connectivity events here if needed
}

esp_err_t matter_meter_init()
{
    ESP_LOGI(tag, "Initializing Electrical Power Measurement cluster");
    ESP_LOGI(tag, "Initializing Matter Node and Electrical Power Measurement cluster");

    // 1. Create the root Matter node (Endpoint 0) if not already created
    node::config_t node_config;
    node_t *node_ptr = node::create(&node_config, nullptr, nullptr);
    if (!node_ptr) {
        ESP_LOGE(tag, "Failed to create Matter root node");
        return ESP_ERR_NO_MEM;
    }


    // 2. Create the Electrical Sensor endpoint on the node
    endpoint::electrical_sensor::config_t sensor_config;

    // Select exactly one Power Topology feature (NodeTopology = single-node device)
    sensor_config.power_topology.feature_flags = cluster::power_topology::feature::node_topology::get_id();

    // Enable the Electrical Power Measurement cluster (O.a+ requirement)
    sensor_config.with_electrical_power_measurement();

    endpoint_t *endpoint_ptr = endpoint::electrical_sensor::create(node_ptr, &sensor_config, ENDPOINT_FLAG_NONE, nullptr);
    if (!endpoint_ptr) {
       ESP_LOGE(tag, "Failed to create electrical sensor endpoint");
       return ESP_ERR_NO_MEM;
    }
    
    s_endpoint_id = endpoint::get_id(endpoint_ptr);

    // 3. Configure the Electrical Power Measurement cluster
    cluster_t *epm_cluster = cluster::get(endpoint_ptr, chip::app::Clusters::ElectricalPowerMeasurement::Id);
    if (epm_cluster) {
        electrical_power_measurement::feature::alternating_current::add(epm_cluster);

        nullable<int64_t> initial_val(0);
        electrical_power_measurement::attribute::create_voltage(epm_cluster, initial_val);
        electrical_power_measurement::attribute::create_active_current(epm_cluster, initial_val);
        electrical_power_measurement::attribute::create_active_power(epm_cluster, initial_val);
    }

  
    // 4. Start the Matter stack (initializes CHIP, Thread/OpenThread, and BLE commissioning)
    esp_err_t err = esp_matter::start(app_event_cb);
    if (err != ESP_OK) {
        ESP_LOGE(tag, "Failed to start Matter stack: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(tag, "Matter Electrical Power Measurement cluster initialized on endpoint %d", s_endpoint_id);
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