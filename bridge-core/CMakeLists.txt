cmake_minimum_required(VERSION 3.20)
project(bridge-core C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(/usr/local/include ${CMAKE_SOURCE_DIR}/inc)
link_directories(/usr/local/lib)

# Core library (object files for now, can be static lib later)
add_library(bridge_core_objs OBJECT
    src/bridge_transport.c
    src/bridge_protocol.c
    src/probe.c
)

# Probe executable
add_executable(bridge_probe src/probe.c)
target_link_libraries(bridge_probe PCBUSB pthread)

# Transport Test executable
add_executable(test_transport 
    test/test_transport.c 
    src/bridge_transport.c
)

# Protocol Test executable
add_executable(test_protocol 
    test/test_protocol.c 
    src/bridge_protocol.c
)
