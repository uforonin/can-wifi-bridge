#!/bin/zsh
source ~/esp/esp-idf/export.sh >/dev/null 2>&1
cd "$(dirname "$0")/.."
idf.py -p /dev/cu.usbmodem101 flash monitor
