#pragma once
#include "esp_err.h"
#include "meter_types.hpp"

esp_err_t matter_meter_init(void);
esp_err_t matter_meter_publish(const meter_snapshot_t &sample);
