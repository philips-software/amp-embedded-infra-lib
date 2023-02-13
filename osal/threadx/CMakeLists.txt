FetchContent_Declare(
    threadx
    GIT_REPOSITORY https://github.com/azure-rtos/threadx
    GIT_TAG e11c166a4045d80ea74fe917e5608474754f5067
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)
set(CMAKE_COMPILE_WARNING_AS_ERROR Off)

if (TARGET_CORTEX)
    set(THREADX_ARCH cortex_${TARGET_CORTEX} CACHE STRING "")
endif()

if (CMAKE_COMPILER_IS_GNUCC)
    set(THREADX_TOOLCHAIN gnu CACHE STRING "")
endif()

FetchContent_MakeAvailable(threadx)

target_compile_definitions(threadx PRIVATE "TX_INITIALIZE_KERNEL_ENTER_EXTENSION=return\;")
get_target_property(COMPILE_DEFINITIONS threadx definitions)
string(REPLACE TX_INCLUDE_USER_DEFINE_FILE TX_INCLUDE_USER_DEFINE_FILE=1 defitions "${definitions}")
set_target_properties(threadx PROPERTIES COMPILE_DEFINITIONS "${definitions}")

add_library(osal.threadx_osal STATIC Osal.cpp)
target_link_libraries(osal.threadx_osal PUBLIC infra.util threadx)

if (TARGET_BUILD_UNIX OR TARGET_BUILD_OSX)
    target_link_libraries(osal.threadx_osal PUBLIC pthread)
endif()
