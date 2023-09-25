add_library(MCU_Support INTERFACE)

add_subdirectory(CMSIS)
add_subdirectory(HAL)
add_subdirectory(linker)
add_subdirectory(startup)

target_link_libraries(MCU_Support INTERFACE
    CMSIS
    HAL
    threadx
)