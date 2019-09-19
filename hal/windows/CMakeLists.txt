ccola_component(windows win: static_library other: skip)

    ccola_sources(
        UartWindows.cpp
        UartWindows.hpp
    )

    ccola_dependencies(
        interfaces
    )

ccola_end_component()
