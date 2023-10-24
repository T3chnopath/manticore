add_library(Common INTERFACE)

add_subdirectory(mcan)
add_subdirectory(utility)

target_link_libraries(Common INTERFACE 
    MCAN
    Utility
)