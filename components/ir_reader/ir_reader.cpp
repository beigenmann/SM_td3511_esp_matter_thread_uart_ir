#include "ir_reader.hpp"
#include "obis_parser.hpp"
#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include <cstring>

static const char *TAG = "ir_reader";
static meter_snapshot_t g_snapshot{};
static bool g_have_snapshot = false;

static uart_port_t uart_port()
{
    return (uart_port_t)CONFIG_SM_TD3511_UART_PORT;
}

esp_err_t ir_reader_init(void)
{
    uart_config_t cfg{};
    cfg.baud_rate = CONFIG_SM_TD3511_UART_BAUD;
    cfg.data_bits = UART_DATA_8_BITS;
#if CONFIG_SM_TD3511_UART_EVEN_PARITY
    cfg.parity = UART_PARITY_EVEN;
#else
    cfg.parity = UART_PARITY_DISABLE;
#endif
#if CONFIG_SM_TD3511_UART_TWO_STOP_BITS
    cfg.stop_bits = UART_STOP_BITS_2;
#else
    cfg.stop_bits = UART_STOP_BITS_1;
#endif
    cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
    cfg.source_clk = UART_SCLK_DEFAULT;

    ESP_ERROR_CHECK(uart_driver_install(uart_port(), 4096, 0, 0, nullptr, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_port(), &cfg));
    ESP_ERROR_CHECK(uart_set_pin(uart_port(),
        CONFIG_SM_TD3511_UART_TX_GPIO < 0 ? UART_PIN_NO_CHANGE : CONFIG_SM_TD3511_UART_TX_GPIO,
        CONFIG_SM_TD3511_UART_RX_GPIO, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_LOGI(TAG, "UART%d ready at %d baud; RX GPIO=%d",
             (int)uart_port(), cfg.baud_rate, CONFIG_SM_TD3511_UART_RX_GPIO);
    return ESP_OK;
}

esp_err_t ir_reader_get_snapshot(meter_snapshot_t *out)
{
    if (!out) return ESP_ERR_INVALID_ARG;

    uint8_t buffer[1024];
    int n = uart_read_bytes(uart_port(), buffer, sizeof(buffer) - 1,
                            pdMS_TO_TICKS(200));
    if (n > 0) {
        buffer[n] = 0;
        if (obis_parse_telegram((const char *)buffer, (size_t)n, &g_snapshot)) {
            g_snapshot.sampled_at_us = esp_timer_get_time();
            g_have_snapshot = true;
        }
    }
    if (!g_have_snapshot) return ESP_ERR_NOT_FOUND;
    *out = g_snapshot;
    return ESP_OK;
}
