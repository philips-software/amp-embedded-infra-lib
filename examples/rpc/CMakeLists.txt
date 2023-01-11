add_executable(examples.rpc_server)
target_sources(examples.rpc_server PRIVATE Server.cpp)
target_link_libraries(examples.rpc_server PRIVATE services.network_instantiations hal.generic)
protocol_buffer_echo_cpp(examples.rpc_server Console.proto)
