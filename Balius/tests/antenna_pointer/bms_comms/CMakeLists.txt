cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/STM32duinoCore.cmake")

set(BOARDNAME "GENERIC_C071GBUX")
if(NOT "${CMAKE_PARENT_LIST_FILE}" STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
  return()
endif()

project(antenna_pointer_bms_comms C CXX ASM)
include(set_board)
set_board("${BOARDNAME}" SERIAL generic USB none)
include(overall_settings)
overall_settings(OPTIMIZATION "s")
include(external_library)
include(build_sketch)
build_sketch(TARGET "bms_comms"
  SOURCES ./bms_comms.ino
  DEPENDS Wire
)
include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/program_swd.cmake")
add_stm32_swd_upload(bms_comms)
add_stm32_dfu_upload(bms_comms)
