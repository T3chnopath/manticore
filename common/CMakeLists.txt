add_library(Common INTERFACE)

add_subdirectory(mcan)
add_subdirectory(utility)
add_subdirectory(bno055)

target_link_libraries(Common INTERFACE 
    MCAN
    Utility
    BNO055
)