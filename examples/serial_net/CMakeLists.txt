ccola_component(serial_net win: executable other: skip)

    ccola_sources(
	win:
        MainWin.cpp
    )

    ccola_dependencies(
        infra.util
		services.util
        hal.generic
	win:
	    services.network_win
		hal.windows
    )

ccola_end_component()
