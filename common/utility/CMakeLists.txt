# Create Library
add_library(Utility INTERFACE)

# Include headers
target_include_directories(Utility INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

