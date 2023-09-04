project(MantiCore LANGUAGES C CXX)
cmake_minimum_required(VERSION 3.22)

set(CMAKE_C_STANDARD 99)

set(MCU "STM32H563")
add_subdirectory(mcu_support)
add_subdirectory(DemoNucleoH563)

set (MCPU                     "-mcpu=Cortex-M33")
set (CMAKE_C_FLAGS "${MCPU} -std=gnu11 ${MFPU} ${MFLOAT_ABI} ${RUNTIME_LIBRARY} -mthumb -Wall -Werror -stdlib=libc")
set (CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp")