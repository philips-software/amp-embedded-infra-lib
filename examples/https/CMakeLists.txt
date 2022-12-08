add_executable(examples.https_client)
target_sources(examples.https_client PRIVATE Client.cpp)
target_link_libraries(examples.https_client PRIVATE examples.network_example hal.generic)
