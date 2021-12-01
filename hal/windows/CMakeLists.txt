if(TARGET_BUILD_WIN)

    add_library(hal.windows EXCLUDE_FROM_ALL STATIC)

    target_link_libraries(hal.windows
        PUBLIC
        hal.interfaces
        setupapi.lib
    )

    target_compile_definitions(hal.windows 
        PUBLIC
        NOMINMAX
        _SETUPAPI_VER=_WIN32_WINNT_LONGHORN
    )

    target_sources(hal.windows PRIVATE
        UartPortFinder.cpp
        UartPortFinder.hpp
        UartWindows.cpp
        UartWindows.hpp
    )

endif(TARGET_BUILD_WIN)