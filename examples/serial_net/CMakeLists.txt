ccola_component(serial_net x86: executable other: skip)

    ccola_sources(
	win:
        MainWin.cpp
    )

    ccola_dependencies(
        infra.util
		services.util
        services.network_win
        hal.generic
	win:
		hal.windows
    )

ccola_end_component()
