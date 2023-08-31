# Create Linker Script Library
add_library(linkerscript INTERFACE)

# Link the linkerscript
target_sources(linkerscript INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/${MCU}xx_FLASH.ld)