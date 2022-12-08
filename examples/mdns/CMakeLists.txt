add_executable(examples.mdns_client)
target_sources(examples.mdns_client PRIVATE Client.cpp)
target_link_libraries(examples.mdns_client PRIVATE examples.network_example hal.generic)

add_executable(examples.mdns_server)
target_sources(examples.mdns_server PRIVATE Server.cpp)
target_link_libraries(examples.mdns_server PRIVATE examples.network_example)
