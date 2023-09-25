add_subdirectory(IT_Handler)
add_subdirectory(BSP)

add_executable(${DEMO}.elf
    main.c
    ${STARTUP_SOURCE}
    ${IT_HANDLER_SOURCE}
)

target_link_options(${DEMO}.elf PRIVATE
    -T ${LINKERSRIPT}
)

target_include_directories(${DEMO}.elf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(${DEMO}.elf
    MCU_Support 
    IT_Handler
    BSP
)