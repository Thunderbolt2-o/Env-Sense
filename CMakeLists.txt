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
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(final_env_sense C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1


add_executable(final_env_sense final_env_sense.cpp
               sx126x/sx126x.c sx126x/sx126x_hal.c sx126x/lora.cpp
               pico-ssd1306/SSD1306_OLED.cpp pico-ssd1306/SSD1306_OLED_font.cpp
               pico-ssd1306/SSD1306_OLED_graphics.cpp pico-ssd1306/SSD1306_OLED_Print.cpp
               bme68x/bme68x_hal.cpp bme68x/bme68x.cpp
               acc/acc.cpp)

pico_set_program_name(final_env_sense "final_env_sense")
pico_set_program_version(final_env_sense "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(final_env_sense 0)
pico_enable_stdio_usb(final_env_sense 1)

# Add the standard library to the build
target_link_libraries(final_env_sense
        pico_stdlib
        )

# Add the standard include files to the build
target_include_directories(final_env_sense PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(final_env_sense
        hardware_spi
        hardware_i2c
        )


pico_add_extra_outputs(final_env_sense)

