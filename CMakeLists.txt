cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD "pico_w")

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(blink C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK:
pico_sdk_init()

add_compile_options(-Wall -Wno-maybe-uninitialized)

# set by BOARD=pico-w
if (NOT PICO_CYW43_SUPPORTED OR NOT TARGET pico_cyw43_arch)
    message(FATAL_ERROR "No cyw43 support!")
endif()

add_executable(blink main.c)

target_link_libraries(blink pico_stdlib pico_cyw43_arch_none)

# create map/bin/hex file etc.
pico_add_extra_outputs(blink)
