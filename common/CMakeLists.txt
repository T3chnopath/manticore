add_library(Common INTERFACE)

add_subdirectory(mcan)

target_link_libraries(Common INTERFACE 
    MCAN)