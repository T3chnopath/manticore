# Create BSP Library
add_library(BSP
    bsp_nucleo_h563.c
)

# Include header
target_include_directories(BSP PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)