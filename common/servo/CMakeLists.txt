# Create Library
add_library(Servo servo.c)

# Link HAL Library
target_link_libraries(Servo Utility MCU_Support)

# Include headers
target_include_directories(Servo PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

