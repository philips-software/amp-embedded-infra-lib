FetchContent_Declare(
    threadx
    GIT_REPOSITORY https://github.com/eclipse-threadx/threadx
    GIT_TAG 06dabb0ad0403f59889ce85eeb3d48dbd9d3759b # v6.4.2_rel
)

set(CMAKE_COMPILE_WARNING_AS_ERROR Off)

if (TARGET_CORTEX)
    set(THREADX_ARCH cortex_${TARGET_CORTEX} CACHE STRING "")
endif()

if (CMAKE_COMPILER_IS_GNUCC)
    set(THREADX_TOOLCHAIN gnu CACHE STRING "")
endif()

set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL On)
FetchContent_MakeAvailable(threadx)
set_directory_properties(PROPERTIES EXCLUDE_FROM_ALL Off)

target_compile_definitions(threadx PUBLIC
    "TX_INITIALIZE_KERNEL_ENTER_EXTENSION=return\\$<SEMICOLON> "
    "TX_SINGLE_MODE_NON_SECURE=1"
)

add_library(osal.threadx_osal ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_sources(osal.threadx_osal PRIVATE
    Osal.cpp
    EventDispatcherThreadX.cpp
    EventDispatcherThreadX.hpp
)

target_link_libraries(osal.threadx_osal PUBLIC infra.util threadx)

if (TARGET_BUILD_UNIX OR TARGET_BUILD_OSX)
    target_link_libraries(osal.threadx_osal PUBLIC pthread)
endif()
