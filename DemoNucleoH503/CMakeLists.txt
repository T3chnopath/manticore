add_subdirectory(IT_Handler)
add_subdirectory(BSP)

add_executable(${BUILD_TARGET}.elf
    main.c
    ${STARTUP_SOURCE}
    ${IT_HANDLER_SOURCE}
)

target_link_options(${BUILD_TARGET}.elf PRIVATE
    -T ${LINKERSRIPT}
)

target_include_directories(${BUILD_TARGET}.elf PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(${BUILD_TARGET}.elf
    MCU_Support 
    Common
    IT_Handler
    BSP
)