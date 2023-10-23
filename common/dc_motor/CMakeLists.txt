# Create Library
add_library(DC_Motor dc_motor.c)

# Link HAL Library
target_link_libraries(DC_Motor Utility MCU_Support)

# Include headers
target_include_directories(DC_Motor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

