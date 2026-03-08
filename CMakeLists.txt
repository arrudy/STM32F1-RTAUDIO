# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(VoiRunner)

file(GLOB SRCS src/*.c)
target_sources(app PRIVATE ${SRCS})


target_compile_options(app PRIVATE
    -ffunction-sections
    -fdata-sections
    -fno-common
)

target_link_options(app PRIVATE
    -Wl,--gc-sections
    -Wl,--print-gc-sections
)

target_link_options(app PUBLIC 
    "-Wl,--defsym,realCoefAQ15=my_realCoefAQ15"
    "-Wl,--defsym,realCoefBQ15=my_realCoefBQ15"
)


target_include_directories(app PRIVATE include)