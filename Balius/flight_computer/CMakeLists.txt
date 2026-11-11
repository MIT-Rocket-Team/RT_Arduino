# Based on the STM32duino easy_cmake.cmake template.
cmake_minimum_required(VERSION 3.21)

# STEP 1: set up the core, libraries, and board.
# -----------------------------------------------------------------------------
include("${CMAKE_CURRENT_LIST_DIR}/../cmake/STM32duinoCore.cmake")

set(BOARDNAME "GENERIC_H743VITX")
resolve_rt_firmware_lib(RT_FIRMWARE_LIB)

if(NOT EXISTS "${RT_FIRMWARE_LIB}/CC1200.cpp")
  message(FATAL_ERROR "RT_Firmware_Libs not found at ${RT_FIRMWARE_LIB}")
endif()

# This file can be included by a larger project without creating a nested project.
if(NOT "${CMAKE_PARENT_LIST_FILE}" STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
  return()
endif()

project(FLIGHT_COMPUTER_project C CXX ASM)

# STEP 2: configure the STM32duino board and build settings.
# -----------------------------------------------------------------------------
include(set_board)
set_board("${BOARDNAME}" SERIAL generic USB none)

include(overall_settings)
overall_settings(
  OPTIMIZATION "s"
  # Newlib Nano is the core default; debug symbols are disabled by default.
)

# STEP 3: configure the sketch and its libraries.
# -----------------------------------------------------------------------------
include(external_library)

include(build_sketch)
build_sketch(TARGET "flight_computer"
  SOURCES
    ./flight_computer.ino
    "${RT_FIRMWARE_LIB}/CC1200.cpp"
    "${RT_FIRMWARE_LIB}/baro.cpp"
    "${RT_FIRMWARE_LIB}/flash.cpp"
    "${RT_FIRMWARE_LIB}/cam.cpp"
    "${RT_FIRMWARE_LIB}/vtx.cpp"
    "${RT_FIRMWARE_LIB}/GPS.cpp"
    "${RT_FIRMWARE_LIB}/power.cpp"
    "${RT_FIRMWARE_LIB}/pyro.cpp"
  DEPENDS
    SPI
)

# Flight computer device drivers and shared firmware types.
target_include_directories(flight_computer PRIVATE "${RT_FIRMWARE_LIB}")

target_compile_options(base_config INTERFACE
  "$<$<COMPILE_LANGUAGE:CXX>:-w>"
)

# STEP 4: optional features and SWD programming.
# -----------------------------------------------------------------------------
include(insights)
insights(TARGET "flight_computer")

find_program(STM32_PROGRAMMER_CLI
  NAMES STM32_Programmer_CLI.exe STM32_Programmer_CLI
  HINTS
    "$ENV{STM32_PRG_PATH}"
    "$ENV{HOME}/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin"
    "$ENV{HOME}/STMicroelectronics/STM32Cube/STM32CubeProgrammer/STM32CubeProgrammer.app/Contents/MacOs/bin"
    "$ENV{HOME}/STMicroelectronics/STM32Cube/STM32CubeProgrammer/STM32CubeProgrammer.app/Contents/Resources/bin"
    "/Applications/STMicroelectronics/STM32Cube/STM32CubeProgrammer/STM32CubeProgrammer.app/Contents/MacOs/bin"
    "/Applications/STMicroelectronics/STM32Cube/STM32CubeProgrammer/STM32CubeProgrammer.app/Contents/Resources/bin"
)
if(STM32_PROGRAMMER_CLI)
  add_custom_target(program_swd
    COMMAND "${STM32_PROGRAMMER_CLI}"
      -c port=SWD
      -w "$<TARGET_FILE_DIR:flight_computer>/flight_computer.bin" 0x08000000
      -v -rst
    DEPENDS flight_computer
    USES_TERMINAL
    COMMENT "Programming STM32H743 over SWD"
  )
endif()
