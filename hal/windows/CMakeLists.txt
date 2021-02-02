ccola_component(windows win: static_library other: skip)

    ccola_inheritable_definitions(
        _SETUPAPI_VER=_WIN32_WINNT_LONGHORN
    )

    ccola_sources(
        UartPortFinder.cpp
        UartPortFinder.hpp
        UartWindows.cpp
        UartWindows.hpp
    )

    ccola_external_libraries(
        setupapi.lib
    )

    ccola_dependencies(
        interfaces
    )

ccola_end_component()
