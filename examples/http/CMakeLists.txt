add_executable(examples.http_server)
target_sources(examples.http_server PRIVATE Server.cpp)
target_link_libraries(examples.http_server PRIVATE services.network_instantiations hal.generic)
