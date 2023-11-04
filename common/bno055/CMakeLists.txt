# Create Library
add_library(BNO055 bno055.c)

# Link Libraries
target_link_libraries(BNO055 MCU_Support)

# Include headers
target_include_directories(BNO055 PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

