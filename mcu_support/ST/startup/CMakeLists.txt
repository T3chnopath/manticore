# Create the Startup Library
if(MCU STREQUAL "STM32H503")
    set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32h503xx.s)
else()
    set(STARTUP_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32h563xx.s)
endif()
message("Selected ${MCU}")