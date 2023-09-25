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

if(BUILD_TARGET STREQUAL "DemoNucleoH503")
    set(MCU "STM32H503")
elseif(BUILD_TARGET STREQUAL "DemoNucleoH563")
    set(MCU "STM32H563")
else()
   message(FATAL_ERROR "No target specified! Use ./scripts/build.sh <DemoNucleoH503 | DemoNucleoH563>")
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
    add_subdirectory(${BUILD_TARGET})
endif()

