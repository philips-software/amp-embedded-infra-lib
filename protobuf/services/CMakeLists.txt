add_library(protobuf.services ${EMIL_EXCLUDE_FROM_ALL} STATIC)
install(TARGETS protobuf.services EXPORT emilProtobufTargets)

protocol_buffer_echo_cpp(protobuf.services ${CMAKE_CURRENT_LIST_DIR}/Network.proto)
protocol_buffer_csharp(protobuf.services ${CMAKE_CURRENT_LIST_DIR}/Network.proto)
protocol_buffer_java(protobuf.services ${CMAKE_CURRENT_LIST_DIR}/Network.proto)

protocol_buffer_echo_cpp(protobuf.services ${CMAKE_CURRENT_LIST_DIR}/Gap.proto)
protocol_buffer_csharp(protobuf.services ${CMAKE_CURRENT_LIST_DIR}/Gap.proto)
protocol_buffer_java(protobuf.services ${CMAKE_CURRENT_LIST_DIR}/Gap.proto)
