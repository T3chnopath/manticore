add_subdirectory(IT_Handler)
add_subdirectory(BSP)
add_executable(Demo main.c)

target_include_directories(Demo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(Demo
    CMSIS
    HAL
    linkerscript
    IT_Handler
    BSP
)
