# Based on the STM32duino easy_cmake.cmake template.
cmake_minimum_required(VERSION 3.21)

# STEP 1: set up the core, libraries, and board.
# -----------------------------------------------------------------------------
# Override either path at configure time when another computer uses a
# different Arduino data directory or sketchbook location.
set(STM32DUINO_CORE_PATH
  "$ENV{LOCALAPPDATA}/Arduino15/packages/RT_STM32/RT_Arduino_Core_STM32"
  CACHE PATH "Path to the installed STM32duino core"
)

if(EXISTS "$ENV{USERPROFILE}/OneDrive/Documents/Arduino/libraries")
  set(_default_user_libs "$ENV{USERPROFILE}/OneDrive/Documents/Arduino/libraries")
else()
  set(_default_user_libs "$ENV{USERPROFILE}/Documents/Arduino/libraries")
endif()
set(ARDUINO_USER_LIBRARIES
  "${_default_user_libs}"
  CACHE PATH "Path to the Arduino sketchbook libraries directory"
)

file(REAL_PATH "${STM32DUINO_CORE_PATH}" CORE_PATH)
file(TO_CMAKE_PATH "${CORE_PATH}" CORE_PATH)
file(REAL_PATH "${ARDUINO_USER_LIBRARIES}" USER_LIBS)
file(TO_CMAKE_PATH "${USER_LIBS}" USER_LIBS)

set(BOARDNAME "GENERIC_C071GBUX")
set(RT_FIRMWARE_LIB "${USER_LIBS}/RT_Firmware_Libs")

if(NOT EXISTS "${CORE_PATH}/cmake/toolchain.cmake")
  message(FATAL_ERROR "STM32duino core not found at ${CORE_PATH}")
endif()
if(NOT EXISTS "${RT_FIRMWARE_LIB}/INA232.cpp")
  message(FATAL_ERROR "RT_Firmware_Libs not found at ${RT_FIRMWARE_LIB}")
endif()

list(APPEND CMAKE_MODULE_PATH "${CORE_PATH}/cmake")
set(CMAKE_TOOLCHAIN_FILE "${CORE_PATH}/cmake/toolchain.cmake")

# This file can be included by a larger project without creating a nested project.
if(NOT "${CMAKE_PARENT_LIST_FILE}" STREQUAL "${CMAKE_CURRENT_LIST_FILE}")
  return()
endif()

project(PWR_project C CXX ASM)

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
build_sketch(TARGET "pwr"
  SOURCES
    ./pwr.ino
    "${RT_FIRMWARE_LIB}/INA232.cpp"
  DEPENDS
    SoftwareSerial
    Wire
)

# INA232 and myTypes are compiled/read from the installed firmware library.
target_include_directories(pwr PRIVATE "${RT_FIRMWARE_LIB}")

# Match the quiet C++ warning setting used by the BMS sketch build.
target_compile_options(base_config INTERFACE
  "$<$<COMPILE_LANGUAGE:CXX>:-w>"
)

# STEP 4: optional features and SWD programming.
# -----------------------------------------------------------------------------
include(insights)
insights(TARGET "pwr")

find_program(STM32_PROGRAMMER_CLI
  NAMES STM32_Programmer_CLI.exe STM32_Programmer_CLI
  HINTS "$ENV{STM32_PRG_PATH}"
)
if(STM32_PROGRAMMER_CLI)
  add_custom_target(program_swd
    COMMAND "${STM32_PROGRAMMER_CLI}" -c port=SWD -w "$<TARGET_FILE_DIR:pwr>/pwr.bin" 0x08000000 -v -rst
    DEPENDS pwr
    USES_TERMINAL
    COMMENT "Programming STM32C071 over SWD"
  )
endif()
