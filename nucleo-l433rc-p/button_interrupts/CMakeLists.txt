cmake_minimum_required(VERSION 3.16)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(button_interrupts)

target_sources(app PRIVATE src/main.c)
