# Create Handler Library
add_library(IT_Handler
    stm32h5xx_it.c
)

target_link_libraries(IT_Handler HAL threadx)

# Include header
target_include_directories(IT_Handler PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

set(IT_HANDLER_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/stm32h5xx_it.c CACHE STRING "IT Handler Source" FORCE)
message(STATUS "IT Handler Source Found: ${IT_HANDLER_SOURCE}")