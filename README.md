# CAN_WiFi_Bridge

Проект моста CAN <-> Wi-Fi на базе ESP32-S3-DevKitC-1.

## Состав

- `bridge-core` — портируемое C11-ядро: CAN device, transport, protocol, model, CLI.
- `bridge-app` — macOS SwiftUI-приложение.
- `firmware/esp32s3` — прошивка ESP32-S3.
- `docs` — протокол, единицы, pinmap, bring-up log.
- `dist` — release-сборки macOS-приложения.

## Базовые параметры

- CAN-адаптер: PCAN-USB
- Драйвер: MacCAN PCBUSB
- Скорость CAN: 500 kbit/s
- Устройство: ESP32-S3-DevKitC-1 + MCP2551
- Роль устройства: CAN-Wi-Fi bridge
