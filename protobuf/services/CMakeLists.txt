add_library(protobuf.services EXCLUDE_FROM_ALL STATIC)

protocol_buffer_echo_cpp(protobuf.services ${CMAKE_CURRENT_LIST_DIR}/Network.proto)
# TODO enable csharp and java generation
# protocol_buffer_csharp(protobuf.services ${CMAKE_CURRENT_LIST_DIR}/Network.proto)
# protocol_buffer_java(protobuf.services ${CMAKE_CURRENT_LIST_DIR}/Network.proto)
