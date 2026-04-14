cmake_minimum_required(VERSION 3.16)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(blinky)

target_sources(app PRIVATE src/main.c)
