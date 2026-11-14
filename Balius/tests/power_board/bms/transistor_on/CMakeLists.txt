cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_LIST_DIR}/../../../../cmake/STM32duinoCore.cmake")

set(BOARDNAME "GENERIC_C071GBUX")
resolve_rt_firmware_lib(RT_FIRMWARE_LIB)
if(NOT EXISTS "${RT_FIRMWARE_LIB}/BQ76922.cpp")
  message(FATAL_ERROR "RT_Firmware_Libs not found at ${RT_FIRMWARE_LIB}")
endif()
if(NOT "${CMAKE_PARENT_LIST_FILE}" STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
  return()
endif()

project(bms_transistor_on C CXX ASM)
include(set_board)
set_board("${BOARDNAME}" SERIAL generic USB none)
include(overall_settings)
overall_settings(OPTIMIZATION "s")
include(external_library)
include(build_sketch)
build_sketch(TARGET "transistor_on"
  SOURCES ./transistor_on.ino "${RT_FIRMWARE_LIB}/BQ76922.cpp"
  DEPENDS Wire
)
target_include_directories(transistor_on PRIVATE "${RT_FIRMWARE_LIB}")
include("${CMAKE_CURRENT_LIST_DIR}/../../../../cmake/program_swd.cmake")
add_stm32_swd_upload(transistor_on)

