# Create BSP Library
add_library(BSP
    bsp_nucleo_h563.c
)

target_link_libraries(BSP MCU_Support)

# Include header
target_include_directories(BSP PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)