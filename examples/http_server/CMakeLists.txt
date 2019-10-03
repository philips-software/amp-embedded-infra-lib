ccola_component(http_server x86: executable other: skip)

    ccola_sources(
        TimeHttpPage.cpp
        TimeHttpPage.hpp
	win:
        MainWin.cpp
	unix:
		MainUnix.cpp
    )

    ccola_dependencies(
        infra.util
        hal.generic
	win:
		services.network_win
	unix:
		services.network_bsd
    )

ccola_end_component()
