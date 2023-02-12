add_executable(examples.https_client)
target_sources(examples.https_client PRIVATE Client.cpp)
target_link_libraries(examples.https_client PRIVATE services.network_instantiations hal.generic)
