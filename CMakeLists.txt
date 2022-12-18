cmake_minimum_required(VERSION 3.12)

set(PICO_BOARD "pico_w")

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico_tests C CXX ASM)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

# Initialize the SDK:
pico_sdk_init()

function(make_exe target src)
    add_executable(${target} ${src})

    target_link_libraries(${target} pico_stdlib pico_cyw43_arch_none)

    # create map/bin/hex file etc.
    pico_add_extra_outputs(${target})
endfunction()

# make_exe(blink src/blink.c)
# make_exe(led_button src/led_button.c)
make_exe(blink_relay src/blink_relay.c)
