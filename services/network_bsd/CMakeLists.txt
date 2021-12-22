if (TARGET_BUILD_UNIX OR TARGET_BUILD_DARWIN)
    add_library(services.network_bsd EXCLUDE_FROM_ALL STATIC)

    target_link_libraries(services.network_bsd PUBLIC services.network)

    target_sources(services.network_bsd PRIVATE
        ConnectionBsd.cpp
        ConnectionBsd.hpp
        DatagramBsd.cpp
        DatagramBsd.hpp
        EventDispatcherWithNetwork.cpp
        EventDispatcherWithNetwork.hpp
        NameLookupBsd.cpp
        NameLookupBsd.hpp
    )
endif()
