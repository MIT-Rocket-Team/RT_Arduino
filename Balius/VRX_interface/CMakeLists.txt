# Based on the STM32duino easy_cmake.cmake template.
cmake_minimum_required(VERSION 3.21)

# STEP 1: set up the core, libraries, and board.
include("${CMAKE_CURRENT_LIST_DIR}/../cmake/STM32duinoCore.cmake")

set(BOARDNAME "GENERIC_C071GBUX")

# This file can be included by a larger project without creating a nested project.
if(NOT "${CMAKE_PARENT_LIST_FILE}" STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
  return()
endif()

project(VRX_interface_project C CXX ASM)

# USB CDC is enabled as the generic Serial port; keep hardware UART support too.
include(set_board)
set_board("${BOARDNAME}" SERIAL generic USB CDCgen)

include(overall_settings)
overall_settings(OPTIMIZATION "s")

include(external_library)
include(build_sketch)
build_sketch(TARGET "VRX_interface"
  SOURCES ./VRX_interface.ino
  DEPENDS USBDevice
)

include(insights)
insights(TARGET "VRX_interface")

# STM32CubeProgrammer's USB1 connection uses the STM32 ROM DFU bootloader.
find_program(STM32_PROGRAMMER_CLI
  NAMES STM32_Programmer_CLI.exe STM32_Programmer_CLI
  HINTS
    "$ENV{STM32_PRG_PATH}"
    "$ENV{HOME}/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin"
    "$ENV{HOME}/STMicroelectronics/STM32Cube/STM32CubeProgrammer/STM32CubeProgrammer.app/Contents/MacOs/bin"
    "$ENV{HOME}/STMicroelectronics/STM32Cube/STM32CubeProgrammer/STM32CubeProgrammer.app/Contents/Resources/bin"
)
if(STM32_PROGRAMMER_CLI)
  add_custom_target(program_dfu
    COMMAND "${STM32_PROGRAMMER_CLI}"
      -c port=USB1
      -w "$<TARGET_FILE_DIR:VRX_interface>/VRX_interface.bin" 0x08000000
      -v -rst
    DEPENDS VRX_interface
    USES_TERMINAL
    COMMENT "Programming VRX_interface over USB DFU"
  )
endif()


