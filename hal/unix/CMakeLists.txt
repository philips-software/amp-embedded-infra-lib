add_library(hal.unix STATIC)
emil_build_for(hal.unix HOST Linux Darwin PREREQUISITE_BOOL EMIL_STANDALONE)

target_link_libraries(hal.unix PUBLIC
    hal.interfaces
)

target_compile_definitions(hal.unix PUBLIC
    EMIL_HAL_UNIX
)

if (EMIL_BUILD_DARWIN)
    target_compile_definitions(hal.unix PRIVATE
        EMIL_OS_DARWIN
    )
endif()

target_sources(hal.unix PRIVATE
    SynchronousUartUnix.cpp
    SynchronousUartUnix.hpp
    UartUnix.cpp
    UartUnix.hpp
    UartUnixBase.cpp
    UartUnixBase.hpp
)
