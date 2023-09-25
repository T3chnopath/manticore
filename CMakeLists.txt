if(NOT SUBTREE)
    cmake_minimum_required(VERSION 3.22)
    project(MantiCore)

    enable_language(C ASM)
    set(CMAKE_SYSTEM_PROCESSOR arm)
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
    set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

    set(CMAKE_C_EXTENSIONS OFF)
    set(CMAKE_C_STANDARD_REQUIRED ON)
endif()

if(MCU STREQUAL "STM32H503")
    message(STATUS "MCU selected: ${MCU}")
    set(DEMO "DemoNucleoH503")
elseif(MCU STREQUAL "STM32H563")
    message(STATUS "MCU selected: ${MCU}")
    set(DEMO "DemoNucleoH563")
else()
    message(FATAL_ERROR "No MCU specified! Use ./scripts/build.sh <STM32H503 | STM32H563>")
endif()

set(MCPU                     "-mcpu=cortex-m33")
set(MFPU                     "-mfpu=fpv5-sp-d16")
set(MFLOAT_ABI               "-mfloat-abi=hard")
set(CMAKE_C_FLAGS "-DCMAKE_BUILD_TYPE=Debug -g3 ${MCPU} ${MFPU} ${MFLOAT_ABI} ${RUNTIME_LIBRARY} -mthumb -Wall -Wno-conversion -Wno-unused-parameter -Wextra --specs=nosys.specs")

add_compile_definitions(USE_HAL_DRIVER=TRUE)
add_compile_definitions(USE_FULL_LL_DRIVER=TRUE)

add_subdirectory(external)
add_subdirectory(mcu_support)
add_subdirectory(common)

if(NOT SUBTREE)
    add_subdirectory(${DEMO})
endif()

