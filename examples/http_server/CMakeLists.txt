ccola_component(http_server win: executable other: skip)

    ccola_sources(
        TimeHttpPage.cpp
        TimeHttpPage.hpp
        MainWin.cpp
    )

    ccola_dependencies(
        infra.util
        services.network_win
        hal.generic
    )

ccola_end_component()

ccola_component(http_server unix: executable other: skip)

    ccola_sources(
        TimeHttpPage.cpp
        TimeHttpPage.hpp
        MainUnix.cpp
    )

    ccola_dependencies(
        infra.util
        services.network_bsd
        hal.generic
    )

ccola_end_component()
