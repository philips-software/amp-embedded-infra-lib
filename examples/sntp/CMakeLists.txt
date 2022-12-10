add_executable(examples.sntp_client)
target_sources(examples.sntp_client PRIVATE Client.cpp)
target_link_libraries(examples.sntp_client PRIVATE services.network_instantiations hal.generic)
