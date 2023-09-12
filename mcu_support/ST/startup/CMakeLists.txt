# Select the Startup ASM Source from the MCU
string(TOLOWER ${MCU} MCU_LOWER)
set(STARTUP_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/startup_${MCU_LOWER}xx.s CACHE STRING "Startup ASM Source" FORCE)

message(STATUS "Selected ${MCU}")
message(STATUS "Startup ASM Source Found: ${STARTUP_SOURCE}")
