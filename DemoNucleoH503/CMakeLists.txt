add_subdirectory(IT_Handler)
add_subdirectory(BSP)

add_executable(Demo.elf
    main.c
    ${STARTUP_SOURCE}
    ${IT_HANDLER_SOURCE}
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
    IT_Handler
    BSP
)