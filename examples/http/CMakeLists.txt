add_executable(examples.http_server)
target_sources(examples.http_server PRIVATE Server.cpp)
target_link_libraries(examples.http_server PRIVATE examples.network_example hal.generic)
