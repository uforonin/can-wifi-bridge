# CAN Protocol

Статус: черновик, до прошивки.

## Параметры

- CAN bitrate: 500 kbit/s
- Device: ESP32-S3 CAN-Wi-Fi bridge
- Endianness: TBD, предварительно little-endian
- Protocol version: обязательна

## Предварительная таблица сообщений

| Direction | CAN ID | Name | DLC | Payload | Period | Note |
|---|---:|---|---:|---|---|---|
| ESP32 -> PC | TBD | DEVICE_STATUS | 8 | TBD | 100 ms | статус |
| PC -> ESP32 | TBD | DEVICE_CMD | 8 | TBD | on request | команда |

## Открытые вопросы

- Какие данные передаются через CAN?
- ESP32 только отвечает на запросы или сам шлёт телеметрию?
- Есть ли несколько логических каналов/потоков?
- Нужны ли мультифреймовые пакеты?
- Нужна ли передача Wi-Fi настроек по CAN?
