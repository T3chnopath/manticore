# Create Library
add_library(MCAN mcan.c)

# Link HAL Library
target_link_libraries(MCAN HAL)

# Include headers
target_include_directories(MCAN PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

