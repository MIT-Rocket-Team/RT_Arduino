# Add an SWD upload target for a sketch built by build_sketch.
function(add_stm32_swd_upload target)
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
        -w "$<TARGET_FILE_DIR:${target}>/${target}.bin" 0x08000000
        -v -rst
      DEPENDS "${target}"
      USES_TERMINAL
      COMMENT "Programming ${BOARDNAME} over SWD"
    )
  endif()
endfunction()
