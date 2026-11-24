cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/STM32duinoCore.cmake")

set(BOARDNAME "GENERIC_C071GBUX")
if(NOT "${CMAKE_PARENT_LIST_FILE}" STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
  return()
endif()

project(antenna_pointer_hello_world C CXX ASM)
include(set_board)
set_board("${BOARDNAME}" SERIAL generic USB CDCgen)
include(overall_settings)
overall_settings(OPTIMIZATION "s")
include(build_sketch)
build_sketch(TARGET "antenna_pointer"
  SOURCES ./antenna_pointer.ino
  DEPENDS USBDevice
)
include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/program_swd.cmake")
add_stm32_swd_upload(antenna_pointer)
add_stm32_dfu_upload(antenna_pointer)
