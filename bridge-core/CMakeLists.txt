cmake_minimum_required(VERSION 3.20)
project(bridge-core C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(/usr/local/include ${CMAKE_SOURCE_DIR}/inc)
link_directories(/usr/local/lib)

add_executable(bridge_probe src/probe.c)
target_link_libraries(bridge_probe PCBUSB pthread)
