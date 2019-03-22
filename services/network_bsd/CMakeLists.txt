ccola_component(network_bsd bsd: static_library other: skip)

    ccola_sources(
        ConnectionBsd.cpp
        ConnectionBsd.hpp
        DatagramBsd.cpp
        DatagramBsd.hpp
        EventDispatcherWithNetwork.cpp
        EventDispatcherWithNetwork.hpp
        NameLookupBsd.cpp
        NameLookupBsd.hpp
    )

    ccola_dependencies(
        services.network
    )

ccola_end_component()
