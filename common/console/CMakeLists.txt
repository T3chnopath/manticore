# Create Library
add_library(Console console.c native_commands.c)

# Link HAL Library
target_link_libraries(Console MCAN MCU_Support)

# Include headers
target_include_directories(Console PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)
