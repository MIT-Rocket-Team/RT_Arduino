cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_LIST_DIR}/../../../../cmake/STM32duinoCore.cmake")

set(BOARDNAME "GENERIC_C071GBUX")
set(RT_FIRMWARE_LIB "${USER_LIBS}/RT_Firmware_Libs")
if(NOT EXISTS "${RT_FIRMWARE_LIB}/BQ76922.cpp")
  message(FATAL_ERROR "RT_Firmware_Libs not found at ${RT_FIRMWARE_LIB}")
endif()
if(NOT "${CMAKE_PARENT_LIST_FILE}" STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
  return()
endif()

project(bms_pwr_serial_test C CXX ASM)
include(set_board)
set_board("${BOARDNAME}" SERIAL generic USB none)
include(overall_settings)
overall_settings(OPTIMIZATION "s")
include(external_library)
include(build_sketch)
build_sketch(TARGET "pwr_serial_test"
  SOURCES ./pwr_serial_test.ino "${RT_FIRMWARE_LIB}/BQ76922.cpp"
  DEPENDS Wire
)
target_include_directories(pwr_serial_test PRIVATE "${RT_FIRMWARE_LIB}")