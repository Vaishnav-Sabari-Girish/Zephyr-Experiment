cmake_minimum_required(VERSION 3.16)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(button_interrupts)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

target_sources(app PRIVATE src/main.c)
