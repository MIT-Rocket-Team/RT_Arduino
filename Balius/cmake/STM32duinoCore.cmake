# Locate an installed official STM32duino core. Set STM32DUINO_CORE_PATH in
# the CMake cache to pin a particular installed version when needed.
if(NOT STM32DUINO_CORE_PATH)
  set(_stm32duino_core_roots)
  if(WIN32 AND DEFINED ENV{LOCALAPPDATA})
    list(APPEND _stm32duino_core_roots
      "$ENV{LOCALAPPDATA}/Arduino15/packages/STMicroelectronics/hardware/stm32"
    )
  endif()
  if(APPLE AND DEFINED ENV{HOME})
    list(APPEND _stm32duino_core_roots
      "$ENV{HOME}/Library/Arduino15/packages/STMicroelectronics/hardware/stm32"
    )
  elseif(UNIX AND DEFINED ENV{HOME})
    list(APPEND _stm32duino_core_roots
      "$ENV{HOME}/.arduino15/packages/STMicroelectronics/hardware/stm32"
    )
  endif()

  set(_stm32duino_core_candidates)
  foreach(_stm32duino_core_root IN LISTS _stm32duino_core_roots)
    file(GLOB _stm32duino_root_candidates LIST_DIRECTORIES TRUE
      "${_stm32duino_core_root}/*"
    )
    list(APPEND _stm32duino_core_candidates ${_stm32duino_root_candidates})
  endforeach()
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
  set(_arduino_library_candidates)
  if(WIN32 AND DEFINED ENV{USERPROFILE})
    list(APPEND _arduino_library_candidates
      "$ENV{USERPROFILE}/OneDrive/Documents/Arduino/libraries"
      "$ENV{USERPROFILE}/Documents/Arduino/libraries"
    )
  elseif(DEFINED ENV{HOME})
    list(APPEND _arduino_library_candidates
      "$ENV{HOME}/Documents/Arduino/libraries"
      "$ENV{HOME}/Arduino/libraries"
    )
  endif()

  foreach(_arduino_library_candidate IN LISTS _arduino_library_candidates)
    if(IS_DIRECTORY "${_arduino_library_candidate}")
      set(ARDUINO_USER_LIBRARIES "${_arduino_library_candidate}")
      break()
    endif()
  endforeach()

  if(NOT ARDUINO_USER_LIBRARIES)
    if(DEFINED ENV{HOME})
      set(ARDUINO_USER_LIBRARIES "$ENV{HOME}/Documents/Arduino/libraries")
    elseif(DEFINED ENV{USERPROFILE})
      set(ARDUINO_USER_LIBRARIES "$ENV{USERPROFILE}/Documents/Arduino/libraries")
    else()
      set(ARDUINO_USER_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/../../libraries")
    endif()
  endif()
endif()

file(REAL_PATH "${ARDUINO_USER_LIBRARIES}" USER_LIBS)
file(TO_CMAKE_PATH "${USER_LIBS}" USER_LIBS)

# Arduino library installs sometimes differ only in the capitalization of this
# directory name. Resolve both common spellings for case-sensitive file systems.
function(resolve_rt_firmware_lib output_variable)
  foreach(_rt_firmware_lib_candidate
      "${USER_LIBS}/RT_Firmware_Libs"
      "${USER_LIBS}/RT_Firmware_libs"
  )
    if(IS_DIRECTORY "${_rt_firmware_lib_candidate}")
      file(REAL_PATH "${_rt_firmware_lib_candidate}" _rt_firmware_lib_path)
      file(TO_CMAKE_PATH "${_rt_firmware_lib_path}" _rt_firmware_lib_path)
      set(${output_variable} "${_rt_firmware_lib_path}" PARENT_SCOPE)
      return()
    endif()
  endforeach()

  set(${output_variable} "${USER_LIBS}/RT_Firmware_Libs" PARENT_SCOPE)
endfunction()

list(APPEND CMAKE_MODULE_PATH "${CORE_PATH}/cmake")
set(CMAKE_TOOLCHAIN_FILE "${CORE_PATH}/cmake/toolchain.cmake")

# The core's CMake download step doesn't know Apple Silicon hosts, so point its
# download cache at the tools Arduino IDE/CLI already installed for this core.
if(CMAKE_HOST_APPLE AND NOT CMAKE_HOST_SYSTEM_PROCESSOR STREQUAL "x86_64")
  string(MD5 _stm32duino_path_hash "${CORE_PATH}/cmake/environment.cmake")
  set(_stm32duino_dist "$ENV{HOME}/.Arduino_Core_STM32_dl/${_stm32duino_path_hash}/dist")
  set(_arduino_tools "$ENV{HOME}/Library/Arduino15/packages")
  foreach(_pair
      "ctags=builtin/tools/ctags"
      "CMSIS6=STMicroelectronics/tools/CMSIS"
      "CMSIS_DSP=STMicroelectronics/tools/CMSIS_DSP"
      "CMSIS_NN=STMicroelectronics/tools/CMSIS_NN"
      "xpack=STMicroelectronics/tools/xpack-arm-none-eabi-gcc")
    string(REPLACE "=" ";" _pair "${_pair}")
    list(GET _pair 0 _name)
    list(GET _pair 1 _tool)
    file(GLOB _installed LIST_DIRECTORIES TRUE "${_arduino_tools}/${_tool}/*")
    if(_installed AND NOT EXISTS "${_stm32duino_dist}/${_name}/.")
      list(SORT _installed COMPARE NATURAL ORDER DESCENDING)
      list(GET _installed 0 _installed)
      file(REMOVE_RECURSE "${_stm32duino_dist}/${_name}")
      file(MAKE_DIRECTORY "${_stm32duino_dist}")
      file(CREATE_LINK "${_installed}" "${_stm32duino_dist}/${_name}" SYMBOLIC)
    endif()
  endforeach()

  # The core's find_program() checks PATH first, which picks Homebrew's
  # arm-none-eabi-gcc (no newlib, so no <stdint.h>). Pin the core's xpack GCC.
  set(_xpack_bin "${_stm32duino_dist}/xpack/bin")
  set(CMAKE_ASM_COMPILER "${_xpack_bin}/arm-none-eabi-gcc" CACHE FILEPATH "")
  set(CMAKE_C_COMPILER "${_xpack_bin}/arm-none-eabi-gcc" CACHE FILEPATH "")
  set(CMAKE_CXX_COMPILER "${_xpack_bin}/arm-none-eabi-g++" CACHE FILEPATH "")
  set(CMAKE_AR "${_xpack_bin}/arm-none-eabi-ar" CACHE FILEPATH "")
  set(CMAKE_LD "${_xpack_bin}/arm-none-eabi-ld" CACHE FILEPATH "")
  set(CMAKE_OBJCOPY "${_xpack_bin}/arm-none-eabi-objcopy" CACHE FILEPATH "")
  set(CMAKE_SIZE "${_xpack_bin}/arm-none-eabi-size" CACHE FILEPATH "")
endif()
