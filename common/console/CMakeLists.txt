# Create Library
add_library(Console console.c)

# Link HAL Library
target_link_libraries(Console MCU_Support)

# Include headers
target_include_directories(Console PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)
