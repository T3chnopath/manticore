# Create Library
add_library(Utility utility.c)

# Include headers
target_include_directories(Utility PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

