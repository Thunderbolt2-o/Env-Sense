# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.0.0)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(http_server C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()
include_directories(${CMAKE_CURRENT_LIST_DIR}/Lib)

# Add executable. Default name is the project name, version 0.1

add_executable(http_server http_server.cpp 
              Lib/WebServer.cpp
              sx126x/sx126x.c sx126x/sx126x_hal.c sx126x/lora.cpp
)


pico_set_program_name(http_server "http_server")
pico_set_program_version(http_server "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(http_server 0)
pico_enable_stdio_usb(http_server 1)

# Add the standard library to the build
target_link_libraries(http_server
        pico_stdlib
        hardware_spi)

# Add the standard include files to the build
target_include_directories(http_server PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(http_server 
       pico_stdlib
       pico_cyw43_arch_lwip_threadsafe_background
        )

pico_add_extra_outputs(http_server)

