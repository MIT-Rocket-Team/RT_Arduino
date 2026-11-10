set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(ARM_GCC_PATH
    "$ENV{LOCALAPPDATA}/Arduino15/packages/STMicroelectronics/tools/xpack-arm-none-eabi-gcc/14.2.1-1.1/bin"
)

set(CMAKE_C_COMPILER   "${ARM_GCC_PATH}/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "${ARM_GCC_PATH}/arm-none-eabi-g++.exe")
set(CMAKE_ASM_COMPILER "${ARM_GCC_PATH}/arm-none-eabi-gcc.exe")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)