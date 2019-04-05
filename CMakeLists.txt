cmake_minimum_required(VERSION 3.12)

project(hpa_fcu)

set(CMAKE_C_STANDARD 99)

# Define some constants
set(AVR_MCU "atmega328p")
set(AVR_SPEED "16000000")
set(AVR_OPT "s")

# Define compiler flags
add_definitions("-mmcu=${AVR_MCU}")
add_definitions("-DF_CPU=${AVR_SPEED}")
add_definitions("-O${AVR_OPT}")
add_definitions("-pedantic")
add_definitions("-Wall")
add_definitions("-Werror")

# Summarize the flags
message("")
message("AVR-GCC OPTIONS ENABLED:")
message("    Target MCU: ${AVR_MCU}")
message("    MCU Speed: ${AVR_SPEED}Hz")
message("    Optimization: -O${AVR_OPT}")
message("")

include_directories(.)
include_directories(feedback)
include_directories(gpio)
include_directories(i2c)
include_directories(lcd)
include_directories(pwm)
include_directories(sensors)

add_executable(main main.c)
