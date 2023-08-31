# Create the Startup Library
if(MCU STREQUAL "STM32H503")
    add_library(startup STATIC ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32h503xx.s)
else()
   add_library(startup STATIC ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32h563xx.s)
endif()