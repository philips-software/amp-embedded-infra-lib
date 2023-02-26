FetchContent_Declare(
    freertos
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel
    GIT_TAG def7d2df2b0506d3d249334974f51e427c17a41c # V10.5.1
)

if (TARGET_CORTEX STREQUAL m7)
    set(FREERTOS_PORT GCC_ARM_CM7 CACHE STRING "" FORCE)
elseif (TARGET_CORTEX STREQUAL m4)
    set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "" FORCE)
else()
    message(FATAL_ERROR "Unknown platform")
endif()

add_library(freertos_config INTERFACE)

target_include_directories(freertos_config INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/config/posix>"
)

target_compile_definitions(freertos_config INTERFACE
    projCOVERAGE_TEST=0
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)
FetchContent_MakeAvailable(freertos)
set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL Off)

set_target_properties(freertos_config freertos_kernel freertos_kernel_port PROPERTIES FOLDER External/FreeRTOS)
add_library(osal.freertos_osal ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_sources(osal.freertos_osal PRIVATE
    LowPowerStrategyFreeRtos.cpp
    LowPowerStrategyFreeRtos.hpp
    Osal.cpp
)

target_link_libraries(osal.freertos_osal PUBLIC
    infra.util
    freertos_kernel
    freertos_kernel_port
)

#install(TARGETS osal.freertos_osal freertos_config freertos_kernel freertos_kernel_port EXPORT emilOsalTargets)

emil_exclude_from_clang_format(freertos_config)
emil_exclude_from_clang_format(freertos_kernel)
emil_exclude_from_clang_format(freertos_kernel_port)
