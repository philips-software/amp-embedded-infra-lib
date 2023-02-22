FetchContent_Declare(
    freertos
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel
    GIT_TAG 90d920466ea2e768e0095ef5a93ff0a0833d9068
)

if (TARGET_CORTEX STREQUAL m7)
    set(FREERTOS_PORT GCC_ARM_CM7 CACHE STRING "" FORCE)
elseif (TARGET_CORTEX STREQUAL m4)
    set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "" FORCE)
else()
    message(FATAL_ERROR "Unknown platform")
endif()

if (CONFIG_FILE_DIRECTORY)
    cmake_path(ABSOLUTE_PATH CONFIG_FILE_DIRECTORY NORMALIZE)
    set(FREERTOS_CONFIG_FILE_DIRECTORY "${CONFIG_FILE_DIRECTORY}" CACHE STRING "" FORCE)
else()
    set(FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/config/posix" CACHE STRING "" FORCE)
endif()

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)
FetchContent_MakeAvailable(freertos)
set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL Off)

set_target_properties(freertos_kernel freertos_kernel_port PROPERTIES FOLDER External/FreeRTOS)
#install(TARGETS freertos_kernel freertos_kernel_port EXPORT emilOsalTargets)

add_library(osal.freertos_osal STATIC ${EMIL_EXCLUDE_FROM_ALL} Osal.cpp)
#install(TARGETS osal.freertos_osal EXPORT emilOsalTargets)
target_link_libraries(osal.freertos_osal PUBLIC infra.util freertos_kernel freertos_kernel_port)

emil_exclude_from_clang_format(freertos_kernel)
emil_exclude_from_clang_format(freertos_kernel_port)
