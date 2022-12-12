add_library(services.network_bsd EXCLUDE_FROM_ALL STATIC)
install(TARGETS services.network_bsd EXPORT emilServicesTargets)

target_link_libraries(services.network_bsd PUBLIC
    services.network
    pthread
)

target_sources(services.network_bsd PRIVATE
    ConnectionBsd.cpp
    ConnectionBsd.hpp
    DatagramBsd.cpp
    DatagramBsd.hpp
    EventDispatcherWithNetwork.cpp
    EventDispatcherWithNetwork.hpp
)

emil_clangformat_target(services.network_bsd)
