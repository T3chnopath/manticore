# Create BSP Library
add_library(BSP
    nucleo_h563.c
)

# Include header
target_include_directories(BSP PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)