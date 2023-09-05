
add_library(CMSIS
    ${CMAKE_CURRENT_SOURCE_DIR}/STM32H5xx/source/system_stm32h5xx.c
)

target_link_libraries(CMSIS HAL)

target_compile_definitions(CMSIS PUBLIC ${MCU}xx=TRUE)

target_include_directories(CMSIS PUBLIC
    $(CMAKE_CURRENT_SOURCE_DIR)/include
    $(CMAKE_CURRENT_SOURCE_DIR)/STM32H5xx/include
)