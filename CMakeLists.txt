cmake_minimum_required(VERSION 3.22)
project(MantiCore)

enable_language(C CXX ASM)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)


set(MCU "STM32H563")
set (MCPU                     "-mcpu=cortex-m33")
set (MFPU                     "-mfpu=fpv5-sp-d16")
set (MFLOAT_ABI               "-mfloat-abi=hard")
set (CMAKE_C_FLAGS "-DCMAKE_BUILD_TYPE=Debug -g3 ${MCPU} ${MFPU} ${MFLOAT_ABI} ${RUNTIME_LIBRARY} -D__ARM_FEATURE_CMSE=3U -std=c99 -mthumb -Wall -Wconversion -Wextra --specs=nosys.specs")

add_compile_definitions(USE_HAL_DRIVER=TRUE)
add_compile_definitions(USE_FULL_LL_DRIVER=TRUE)

add_subdirectory(mcu_support)
add_subdirectory(DemoNucleoH563)

