add_library(CMSIS INTERFACE)

target_include_directories(CMSIS INTERFACE 
    $(CMAKE_CURRENT_SOURCE_DIR)/include
    $(CMAKE_CURRENT_SOURCE_DIR)/STM32H5xx/include
)