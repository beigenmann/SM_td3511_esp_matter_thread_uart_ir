#pragma once
#include "esp_err.h"
#include "meter_types.hpp"
#ifdef __cplusplus
extern "C" {
#endif
esp_err_t ir_reader_init(void);
esp_err_t ir_reader_get_snapshot(meter_snapshot_t *out);
#ifdef __cplusplus
}
#endif
