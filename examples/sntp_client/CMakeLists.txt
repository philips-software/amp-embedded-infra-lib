ccola_component(sntp_client win: executable other: skip)

    ccola_sources(
        MainWin.cpp
    )

    ccola_dependencies(
        infra.util
		services.util
        services.network_win
        hal.generic
    )

ccola_end_component()
