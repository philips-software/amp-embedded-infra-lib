ccola_component(http_server win: executable other: skip)

    ccola_sources(
        Main.cpp
    )

    ccola_dependencies(
        infra.util
        services.network_win
        hal.windows
    )

ccola_end_component()
