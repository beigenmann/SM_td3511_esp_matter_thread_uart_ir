#pragma once
#include <stddef.h>
#include "meter_types.hpp"

// Parse a buffer containing one or more line-oriented OBIS records.
// Returns true if at least one recognized field was updated.
bool obis_parse_telegram(const char *data, size_t length, meter_snapshot_t *out);
