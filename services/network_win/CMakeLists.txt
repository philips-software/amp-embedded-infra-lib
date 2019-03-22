ccola_component(network_win win: static_library other: skip)

    ccola_sources(
        ConnectionWin.cpp
        ConnectionWin.hpp
        DatagramWin.cpp
        DatagramWin.hpp
        EventDispatcherWithNetwork.cpp
        EventDispatcherWithNetwork.hpp
        NameLookupWin.cpp
        NameLookupWin.hpp
    )

    ccola_dependencies(
        services.network
    )
        
    ccola_external_libraries(
        ws2_32.lib
    )

ccola_end_component()
