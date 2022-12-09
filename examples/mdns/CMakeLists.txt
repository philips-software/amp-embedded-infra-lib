add_executable(examples.mdns_client)
target_sources(examples.mdns_client PRIVATE Client.cpp)
target_link_libraries(examples.mdns_client PRIVATE services.network_instantiations hal.generic)

add_executable(examples.mdns_server)
target_sources(examples.mdns_server PRIVATE Server.cpp)
target_link_libraries(examples.mdns_server PRIVATE services.network_instantiations)
