# Create Handler Library
add_library(IT_Handler
    stm32h5xx_it.c
)

# Include header
target_include_directories(IT_Handler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)