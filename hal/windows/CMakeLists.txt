if (TARGET_BUILD_WIN)
    add_library(hal.windows ${EMIL_EXCLUDE_FROM_ALL} STATIC)
    install(TARGETS hal.windows EXPORT emilHalTargets)

    target_link_libraries(hal.windows PUBLIC
        hal.interfaces
        setupapi.lib
    )

    target_compile_definitions(hal.windows PUBLIC
        NOMINMAX
        _SETUPAPI_VER=_WIN32_WINNT_LONGHORN
    )

    target_sources(hal.windows PRIVATE
        SynchronousUartWindows.cpp
        SynchronousUartWindows.hpp
        UartPortFinder.cpp
        UartPortFinder.hpp
        UartWindows.cpp
        UartWindows.hpp
    )
endif()
