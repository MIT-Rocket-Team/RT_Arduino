# Locate an installed official STM32duino core. Set STM32DUINO_CORE_PATH in
# the CMake cache to pin a particular installed version when needed.
if(NOT STM32DUINO_CORE_PATH)
  file(GLOB _stm32duino_core_candidates LIST_DIRECTORIES TRUE
    "$ENV{LOCALAPPDATA}/Arduino15/packages/STMicroelectronics/hardware/stm32/*"
  )
  list(SORT _stm32duino_core_candidates COMPARE NATURAL ORDER DESCENDING)

  foreach(_stm32duino_candidate IN LISTS _stm32duino_core_candidates)
    if(EXISTS "${_stm32duino_candidate}/cmake/toolchain.cmake"
       AND EXISTS "${_stm32duino_candidate}/boards.txt")
      set(STM32DUINO_CORE_PATH
        "${_stm32duino_candidate}"
        CACHE PATH "Path to the installed official STM32duino core"
      )
      break()
    endif()
  endforeach()
endif()

if(NOT STM32DUINO_CORE_PATH
   OR NOT EXISTS "${STM32DUINO_CORE_PATH}/cmake/toolchain.cmake"
   OR NOT EXISTS "${STM32DUINO_CORE_PATH}/boards.txt")
  message(FATAL_ERROR
    "Official STM32duino core not found. Install it with Arduino Board Manager "
    "or configure with -DSTM32DUINO_CORE_PATH=<core directory>."
  )
endif()

file(REAL_PATH "${STM32DUINO_CORE_PATH}" CORE_PATH)
file(TO_CMAKE_PATH "${CORE_PATH}" CORE_PATH)

if(NOT ARDUINO_USER_LIBRARIES)
  if(EXISTS "$ENV{USERPROFILE}/OneDrive/Documents/Arduino/libraries")
    set(_default_arduino_libraries "$ENV{USERPROFILE}/OneDrive/Documents/Arduino/libraries")
  else()
    set(_default_arduino_libraries "$ENV{USERPROFILE}/Documents/Arduino/libraries")
  endif()
  set(ARDUINO_USER_LIBRARIES "${_default_arduino_libraries}")
endif()

file(REAL_PATH "${ARDUINO_USER_LIBRARIES}" USER_LIBS)
file(TO_CMAKE_PATH "${USER_LIBS}" USER_LIBS)

list(APPEND CMAKE_MODULE_PATH "${CORE_PATH}/cmake")
set(CMAKE_TOOLCHAIN_FILE "${CORE_PATH}/cmake/toolchain.cmake")
