#pragma once
#include <stdint.h>

typedef struct {
    float voltage_v = 0.0f;
    float current_a = 0.0f;
    float active_power_w = 0.0f;
    float frequency_hz = 0.0f;
    float power_factor = 0.0f;
    uint64_t imported_energy_wh = 0;
    uint64_t exported_energy_wh = 0;
    bool voltage_valid = false;
    bool current_valid = false;
    bool power_valid = false;
    bool frequency_valid = false;
    bool power_factor_valid = false;
    bool imported_energy_valid = false;
    bool exported_energy_valid = false;
} meter_phase_t;

typedef struct {
    meter_phase_t l1;
    meter_phase_t l2;
    meter_phase_t l3;
    float total_active_power_w = 0.0f;
    bool total_power_valid = false;
    uint64_t total_imported_energy_wh = 0;
    uint64_t total_exported_energy_wh = 0;
    bool total_imported_energy_valid = false;
    bool total_exported_energy_valid = false;
    int64_t sampled_at_us = 0;
} meter_snapshot_t;
