ccola_component(http_server win: executable other: skip)

    ccola_sources(
        TimeHttpPage.cpp
        TimeHttpPage.hpp
        MainWin.cpp
    )

    ccola_dependencies(
        infra.util
        services.network_win
        hal.windows
    )

ccola_end_component()

ccola_component(http_server bsd: executable other: skip)

    ccola_sources(
        TimeHttpPage.cpp
        TimeHttpPage.hpp
        MainBsd.cpp
    )

    ccola_dependencies(
        infra.util
        services.network_bsd
        hal.bsd
    )

ccola_end_component()
