cmake_minimum_required(VERSION 3.21)

# STM32duino core installed by Arduino IDE / Board Manager.
file(REAL_PATH
  "$ENV{LOCALAPPDATA}/Arduino15/packages/RT_STM32/RT_Arduino_Core_STM32"
  CORE_PATH
)
file(REAL_PATH
  "$ENV{USERPROFILE}/OneDrive/Documents/Arduino/libraries/RT_Firmware_Libs"
  RT_FIRMWARE_LIB
)

if(NOT EXISTS "${CORE_PATH}/cmake/toolchain.cmake")
  message(FATAL_ERROR "STM32duino core not found at ${CORE_PATH}")
endif()
if(NOT EXISTS "${RT_FIRMWARE_LIB}/BQ76922.cpp")
  message(FATAL_ERROR "RT_Firmware_Libs not found at ${RT_FIRMWARE_LIB}")
endif()

list(APPEND CMAKE_MODULE_PATH "${CORE_PATH}/cmake")
set(CMAKE_TOOLCHAIN_FILE "${CORE_PATH}/cmake/toolchain.cmake")

project(BMS C CXX ASM)

include(set_board)
set_board(GENERIC_C071GBUX SERIAL generic USB none)

include(overall_settings)
overall_settings(OPTIMIZATION s)

include(build_sketch)
build_sketch(TARGET bms
  SOURCES
    bms.ino
    "${RT_FIRMWARE_LIB}/BQ76922.cpp"
)
target_include_directories(bms PRIVATE "${RT_FIRMWARE_LIB}")
target_link_libraries(bms PRIVATE Wire)

# Optional upload helper; the compile itself does not require CubeProgrammer.
find_program(STM32_PROGRAMMER_CLI
  NAMES STM32_Programmer_CLI.exe STM32_Programmer_CLI
  HINTS "$ENV{STM32_PRG_PATH}"
)
if(STM32_PROGRAMMER_CLI)
  add_custom_target(program_swd
    COMMAND "${STM32_PROGRAMMER_CLI}" -c port=SWD -w "$<TARGET_FILE_DIR:bms>/bms.bin" 0x08000000 -v -rst
    DEPENDS bms
    USES_TERMINAL
    COMMENT "Programming STM32C071 over SWD"
  )
endif()
