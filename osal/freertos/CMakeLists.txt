FetchContent_Declare(
    freertos
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel
    GIT_TAG 8252edec4547afed4e6ada74bd9353b52972894f
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

add_library(freertos_config INTERFACE)

target_include_directories(freertos_config INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/config/posix>"
)

target_compile_definitions(freertos_config INTERFACE
    projCOVERAGE_TEST=0
)

if (CMAKE_COMPILER_IS_GNUCC)
    if (TARGET_CORTEX STREQUAL m7)
        set(FREERTOS_PORT GCC_ARM_CM7 CACHE STRING "")
    elseif (TARGET_CORTEX STREQUAL m4)
        set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "")
    endif()
endif()

FetchContent_MakeAvailable(freertos)

set_target_properties(freertos_config freertos_kernel freertos_kernel_port PROPERTIES FOLDER External/FreeRTOS)

add_library(osal.freertos_osal STATIC Osal.cpp)

target_link_libraries(osal.freertos_osal PUBLIC
    infra.util
    freertos_kernel
    freertos_kernel_port
)

install(TARGETS osal.freertos_osal freertos_config freertos_kernel freertos_kernel_port EXPORT emilOsalTargets)
