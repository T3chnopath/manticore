# Create the Startup Library
if(MCU STREQUAL "STM32H503")
    set(STARTUP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32h503xx.s CACHE STRING "Startup file" FORCE)
else()
    set(STARTUP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32h563xx.s CACHE STRING "Startup file" FORCE)
endif()
message("Selected ${MCU}")