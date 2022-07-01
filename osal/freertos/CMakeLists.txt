FetchContent_Declare(
    freertos
    GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel
    GIT_TAG 90d920466ea2e768e0095ef5a93ff0a0833d9068
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

if (TARGET_BUILD_UNIX OR TARGET_BUILD_OSX)
    cmake_path(SET CONFIG_FILE_DIRECTORY config/posix)
    set(FREERTOS_PORT GCC_POSIX CACHE STRING "")
endif()

if (CONFIG_FILE_DIRECTORY)
    cmake_path(ABSOLUTE_PATH CONFIG_FILE_DIRECTORY NORMALIZE)
    set(FREERTOS_CONFIG_FILE_DIRECTORY "${CONFIG_FILE_DIRECTORY}" CACHE STRING "")
endif()

FetchContent_MakeAvailable(freertos)

set_target_properties(freertos_kernel freertos_kernel_port PROPERTIES FOLDER External/FreeRTOS)
#install(TARGETS freertos_kernel freertos_kernel_port EXPORT emilOsalTargets)

add_library(osal.freertos_osal STATIC Osal.cpp)
target_link_libraries(osal.freertos_osal PUBLIC infra.util freertos_kernel freertos_kernel_port)

if (TARGET_BUILD_UNIX OR TARGET_BUILD_OSX)
    target_link_libraries(osal.freertos_osal PUBLIC pthread)
endif()
