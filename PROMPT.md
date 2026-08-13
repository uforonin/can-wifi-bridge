# Project Prompt

Работаем по двухслойной архитектуре: C11 core + SwiftUI app.

## Правила

- Ядро не знает про GUI.
- GUI — тонкий слой над C API.
- Каждый модуль ядра имеет юнит-тесты.
- Живое железо проверяется CLI-стендом до написания GUI.
- Коммиты атомарные: этап + суть.
- Выводы терминала читать дословно; числа в дампах — источник правды.
- Если данных недостаточно — не угадывать, а запрашивать у инженера.

## Текущие параметры проекта

- Project: CAN_WiFi_Bridge
- Core prefix: bridge_
- CAN adapter: PCAN-USB
- Driver: MacCAN PCBUSB
- CAN bitrate: 500 kbit/s
- Device: ESP32-S3-DevKitC-1
- CAN transceiver: MCP2551
- Device role: CAN-Wi-Fi bridge
