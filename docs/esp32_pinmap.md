# ESP32-S3 Pinmap

Board: ESP32-S3-Zero (V1856)
Chip: ESP32-S3 (QFN56), rev v0.2, Flash 4MB (XMC), PSRAM 2MB
USB: встроенный USB Serial/JTAG, /dev/cu.usbmodem101
Прошивка: BOOT+RESET перед flash, либо power cycle
MAC: b4:3a:45:ae:69:c0

## Проверено
- RGB LED WS2812: GPIO48 (мигает синим)
- Кнопки: BOOT (GPIO0), RESET
- Wi-Fi softAP: SSID=Bridge_AP, pass=bridge123, ch=1, IP=192.168.4.1
- DHCP: клиент получает 192.168.4.2, станция join/leave видна в логе

## CAN (TWAI) — предварительно, этап 2+
- TWAI_TX: GPIO17
- TWAI_RX: GPIO18
- Трансивер: MCP2551 (подключить позже; учесть уровни 3.3V)
- Терминация: 120 Ом на концах шины
