add_library(Common INTERFACE)

add_subdirectory(utility)
add_subdirectory(bno055)
add_subdirectory(console)
add_subdirectory(dc_motor)
add_subdirectory(mcan)
add_subdirectory(servo)

target_link_libraries(Common INTERFACE 
    Utility 
    BNO055
    Console
    DC_Motor
    MCAN
    Servo
)