cmake_minimum_required(VERSION 3.21)
include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/STM32duinoCore.cmake")

set(BOARDNAME "GENERIC_H742VITX")
resolve_rt_firmware_lib(RT_FIRMWARE_LIB)
if(NOT EXISTS "${RT_FIRMWARE_LIB}/flash.cpp")
  message(FATAL_ERROR "RT_Firmware_Libs not found at ${RT_FIRMWARE_LIB}")
endif()
if(NOT "${CMAKE_PARENT_LIST_FILE}" STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
  return()
endif()

project(flight_computer_flash_test C CXX ASM)
include(set_board)
set_board("${BOARDNAME}" SERIAL generic USB none)
include(overall_settings)
overall_settings(OPTIMIZATION "s")
include(external_library)
include(build_sketch)
# Give the UBX-NAV-PVT parser enough RX capacity for a complete packet.
target_compile_definitions(base_config INTERFACE SERIAL_RX_BUFFER_SIZE=128)
build_sketch(TARGET "flash_test"
  SOURCES ./flash_test.ino "${RT_FIRMWARE_LIB}/flash.cpp"
  DEPENDS SPI
)
target_include_directories(flash_test PRIVATE "${RT_FIRMWARE_LIB}")
include("${CMAKE_CURRENT_LIST_DIR}/../../../cmake/program_swd.cmake")
add_stm32_swd_upload(flash_test)
