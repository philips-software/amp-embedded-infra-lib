FetchContent_Declare(
    threadx
    GIT_REPOSITORY https://github.com/philips-forks/threadx
    GIT_TAG 83b57acde9dddf1c9468f0f3f0bed7a764f73c85
)

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)

if (TARGET_BUILD_UNIX AND CMAKE_COMPILER_IS_GNUCC)
    set(THREADX_ARCH linux CACHE STRING "")
    set(THREADX_TOOLCHAIN gnu CACHE STRING "")
endif()

if (TARGET_BUILD_WIN AND MSVC)
    set(THREADX_ARCH win32 CACHE STRING "")
    set(THREADX_TOOLCHAIN vs_2019 CACHE STRING "")
endif()

FetchContent_MakeAvailable(threadx)

target_compile_definitions(threadx PRIVATE TX_INITIALIZE_KERNEL_ENTER_EXTENSION=return\;)

add_library(osal.threadx_osal STATIC Osal.cpp)
target_link_libraries(osal.threadx_osal PUBLIC infra.util threadx)

if (TARGET_BUILD_UNIX OR TARGET_BUILD_OSX)
    target_link_libraries(osal.threadx_osal PUBLIC pthread)
endif()
