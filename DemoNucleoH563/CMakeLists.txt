add_subdirectory(IT_Handler)
add_subdirectory(BSP)

add_executable(${TARGET}.elf
    main.c
    ${STARTUP_SOURCE}
    ${IT_HANDLER_SOURCE}
)

target_link_options(${TARGET}.elf PRIVATE
    -T ${LINKERSRIPT}
)

target_include_directories(${TARGET}.elf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(${TARGET}.elf
    MCU_Support 
    IT_Handler
    BSP
)