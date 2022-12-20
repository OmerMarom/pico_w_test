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
    pico_add_extra_outputs(${target})
endfunction()

# make_exe(blink src/blink.cpp)
# target_link_libraries(blink pico_stdlib pico_cyw43_arch_none)

# make_exe(blink_relay src/blink_relay.cpp)
# target_link_libraries(blink_relay pico_stdlib)

# make_exe(led_button src/led_button.cpp)
# target_link_libraries(led_button pico_stdlib)

make_exe(tcp_client src/tcp_client/main.cpp)
target_include_directories(tcp_client PRIVATE src/tcp_client)
target_link_libraries(tcp_client pico_stdlib pico_cyw43_arch_lwip_threadsafe_background)
