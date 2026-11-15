# SM_td3511 — ESP32-C6 / ESP-Matter / Thread / UART IR reader

Starter project for a three-phase meter. Measurements are received from an optical IR reader
over UART (for example, a meter's IEC 62056-21/D0-style optical interface), then exposed
through a Matter adapter.

**Important:** this is a hardware-integration scaffold. The exact meter telegram and protocol
mode must be configured for the specific electricity meter. The parser included here handles
common line-oriented OBIS text records such as `/...`, `1-0:1.8.0(001234.567*kWh)` and
`1-0:32.7.0(230.1*V)`, but should be validated against captured real telegrams. It is not a
complete IEC 62056-21 state-machine implementation and does not implement encrypted DLMS/COSEM.

## UART defaults

Set the UART port, TX/RX pins, baud rate, parity and stop bits to match the ESP32-C6 board and
your optical reader. Many meters use 300 baud during IEC 62056-21 sign-on and then switch to a
higher baud rate, while some IR readers expose a fixed 9600/8N1 UART. These are not interchangeable.
The defaults here (UART1, 9600 8N1) are only examples.

Configure via `idf.py menuconfig` under `SM_td3511 IR UART reader`.

## Build

```bash
idf.py set-target esp32c6
idf.py menuconfig
idf.py build
idf.py -p PORT flash monitor
```

This requires a compatible ESP-IDF + ESP-Matter checkout. The Matter cluster adapter remains
SDK-version-specific and is intentionally isolated in `main/matter_meter.cpp`.

## Parsed values

Parser recognizes common OBIS identifiers:
- `1-0:32.7.0` voltage L1
- `1-0:52.7.0` voltage L2
- `1-0:72.7.0` voltage L3
- `1-0:31.7.0` current L1
- `1-0:51.7.0` current L2
- `1-0:71.7.0` current L3
- `1-0:1.7.0` active import power (meter-specific phase/total interpretation)
- `1-0:1.8.0` cumulative imported energy
- `1-0:2.8.0` cumulative exported energy
- `1-0:14.7.0` frequency (not all meters provide this)
- `1-0:13.7.0` power factor (not all meters provide this)

OBIS IDs and units vary by meter. Verify the meter's documentation and a real telegram before
relying on these mappings. Active power and energy may be total values, not per-phase values.
The parser deliberately does not copy total power/energy into all three phases.

## Safety / accuracy

Use an isolated, electrically appropriate optical reader. Confirm UART logic levels (3.3 V TTL
versus RS-232 or other interfaces) before wiring to the ESP32-C6. Do not connect mains signals
directly to UART pins.
