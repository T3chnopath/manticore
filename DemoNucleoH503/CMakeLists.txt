add_subdirectory(IT_Handler)
add_subdirectory(BSP)

add_executable(Demo.elf
    main.c
    ${STARTUP}
    ${CMAKE_CURRENT_SOURCE_DIR}/IT_Handler/stm32h5xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../external/threadx/common/src/tx_initialize_kernel_enter.c
)

target_link_options(Demo.elf PRIVATE
    -T ${LINKERSRIPT}
)

target_include_directories(Demo.elf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(Demo.elf
    threadx
    CMSIS
    HAL
    BSP
)