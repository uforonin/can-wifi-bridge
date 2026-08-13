# ESP32-S3 Pinmap

Device: ESP32-S3-DevKitC-1
CAN transceiver: MCP2551
CAN bitrate: 500 kbit/s

## Warning

MCP2551 часто рассчитан на 5 V.
ESP32-S3 GPIO — 3.3 V.

Перед подключением проверить уровни RXD.
При необходимости использовать level shifter или делитель.

## TWAI GPIO

- TWAI_TX_GPIO: TBD
- TWAI_RX_GPIO: TBD

## MCP2551 connections

- VCC: TBD
- GND: common with ESP32 and PCAN-USB
- CANH: to PCAN-USB CANH
- CANL: to PCAN-USB CANL
- Termination: 120 Ohm at both bus ends, if physical layout requires it
