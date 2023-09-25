add_subdirectory(IT_Handler)
add_subdirectory(BSP)

set(EXECUTABLE "${DEMO}.elf")

add_executable(${EXECUTABLE}
    main.c
    ${STARTUP_SOURCE}
    ${IT_HANDLER_SOURCE}
)

target_link_options(${EXECUTABLE} PRIVATE
    -T ${LINKERSRIPT}
)

target_include_directories(${EXECUTABLE} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(${EXECUTABLE}
    MCU_Support 
    IT_Handler
    BSP
)