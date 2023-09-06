add_subdirectory(IT_Handler)
add_subdirectory(BSP)

add_executable(Demo.elf
    main.c
    startup_stm32h563xx.s
    ${CMAKE_CURRENT_SOURCE_DIR}/IT_Handler/stm32h5xx_it.c
)

target_link_options(Demo.elf PRIVATE
    -T ${CMAKE_CURRENT_SOURCE_DIR}/STM32H563xx_FLASH.ld
)

target_include_directories(Demo.elf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(Demo.elf
    CMSIS
    HAL
    BSP
)