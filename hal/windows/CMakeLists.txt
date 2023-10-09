add_library(hal.windows STATIC)
emil_build_for(hal.windows HOST Windows PREREQUISITE_BOOL EMIL_STANDALONE)

target_link_libraries(hal.windows PUBLIC
    hal.interfaces
    setupapi.lib
)

target_compile_definitions(hal.windows PUBLIC
    _SETUPAPI_VER=_WIN32_WINNT_LONGHORN
    EMIL_HAL_WINDOWS
    NOMINMAX
)

target_sources(hal.windows PRIVATE
    SynchronousUartWindows.cpp
    SynchronousUartWindows.hpp
    UartPortFinder.cpp
    UartPortFinder.hpp
    UartWindows.cpp
    UartWindows.hpp
)
