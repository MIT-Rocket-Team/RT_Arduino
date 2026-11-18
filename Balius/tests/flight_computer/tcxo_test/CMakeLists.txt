cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/STM32duinoCore.cmake")

set(BOARDNAME "GENERIC_H742VITX")
if(NOT "${CMAKE_PARENT_LIST_FILE}" STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
  return()
endif()

add_compile_definitions(HSE_VALUE=39000000U)

project(flight_computer_tcxo_test C CXX ASM)
include(set_board)
set_board("${BOARDNAME}" SERIAL generic USB none)
include(overall_settings)
overall_settings(OPTIMIZATION "s")
include(external_library)
include(build_sketch)
build_sketch(TARGET "tcxo_test"
  SOURCES ./tcxo_test.ino ./custom_clock.c
)


include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/program_swd.cmake")
add_stm32_swd_upload(tcxo_test)
