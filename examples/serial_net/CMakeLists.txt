ccola_component(serial_net win: executable other: skip)

    ccola_add_version_header_dependency()

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
