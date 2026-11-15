#include "obis_parser.hpp"
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>
#include <algorithm>

static bool extract_obis_value(const char *line, char *obis, size_t obis_size,
                               double *value, char *unit, size_t unit_size)
{
    const char *colon = std::strchr(line, ':');
    const char *open = std::strchr(line, '(');
    const char *close = std::strchr(line, ')');
    if (!colon || !open || !close || open <= colon || close <= open) return false;

    const char *start = line;
    while (*start == '/' || *start == ' ' || *start == '\t') ++start;
    size_t id_len = (size_t)(open - start);
    if (id_len == 0 || id_len >= obis_size) return false;
    std::memcpy(obis, start, id_len);
    obis[id_len] = '\0';

    char inside[64]{};
    size_t n = (size_t)(close - open - 1);
    if (n == 0 || n >= sizeof(inside)) return false;
    std::memcpy(inside, open + 1, n);
    inside[n] = '\0';

    char *star = std::strchr(inside, '*');
    if (star) {
        *star++ = '\0';
        std::snprintf(unit, unit_size, "%s", star);
    } else {
        unit[0] = '\0';
    }

    char *end = nullptr;
    *value = std::strtod(inside, &end);
    return end != inside;
}

static uint64_t energy_wh(double value, const char *unit)
{
    if (std::strcmp(unit, "kWh") == 0) return (uint64_t)(value * 1000.0 + 0.5);
    if (std::strcmp(unit, "Wh") == 0) return (uint64_t)(value + 0.5);
    return 0;
}

bool obis_parse_telegram(const char *data, size_t length, meter_snapshot_t *out)
{
    if (!data || !out) return false;
    bool changed = false;
    size_t pos = 0;
    while (pos < length) {
        size_t end = pos;
        while (end < length && data[end] != '\n' && data[end] != '\r') ++end;
        char line[160]{};
        size_t n = std::min(end - pos, sizeof(line) - 1);
        std::memcpy(line, data + pos, n);
        pos = end + 1;
        if (n == 0) continue;

        char id[48]{}, unit[16]{};
        double v = 0.0;
        if (!extract_obis_value(line, id, sizeof(id), &v, unit, sizeof(unit))) continue;

        auto voltage = [&](meter_phase_t &p) { p.voltage_v = (float)v; p.voltage_valid = true; changed = true; };
        auto current = [&](meter_phase_t &p) { p.current_a = (float)v; p.current_valid = true; changed = true; };

        if (!std::strcmp(id, "1-0:32.7.0")) voltage(out->l1);
        else if (!std::strcmp(id, "1-0:52.7.0")) voltage(out->l2);
        else if (!std::strcmp(id, "1-0:72.7.0")) voltage(out->l3);
        else if (!std::strcmp(id, "1-0:31.7.0")) current(out->l1);
        else if (!std::strcmp(id, "1-0:51.7.0")) current(out->l2);
        else if (!std::strcmp(id, "1-0:71.7.0")) current(out->l3);
        else if (!std::strcmp(id, "1-0:1.7.0")) {
            out->total_active_power_w = (float)v;
            if (!std::strcmp(unit, "kW")) out->total_active_power_w *= 1000.0f;
            out->total_power_valid = true;
            changed = true;
        }
        else if (!std::strcmp(id, "1-0:1.8.0")) {
            out->total_imported_energy_wh = energy_wh(v, unit);
            out->total_imported_energy_valid = (std::strcmp(unit, "kWh") == 0 || std::strcmp(unit, "Wh") == 0);
            changed = changed || out->total_imported_energy_valid;
        }
        else if (!std::strcmp(id, "1-0:2.8.0")) {
            out->total_exported_energy_wh = energy_wh(v, unit);
            out->total_exported_energy_valid = (std::strcmp(unit, "kWh") == 0 || std::strcmp(unit, "Wh") == 0);
            changed = changed || out->total_exported_energy_valid;
        }
        else if (!std::strcmp(id, "1-0:14.7.0")) {
            // Meter-specific frequency field; unit must be checked against real telegram.
            out->l1.frequency_hz = (float)v;
            out->l1.frequency_valid = true;
            changed = true;
        }
        else if (!std::strcmp(id, "1-0:13.7.0")) {
            out->l1.power_factor = (float)v;
            out->l1.power_factor_valid = true;
            changed = true;
        }
    }
    return changed;
}
