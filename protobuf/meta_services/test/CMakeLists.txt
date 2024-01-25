add_executable(protobuf.meta_services_test)
emil_build_for(protobuf.meta_services_test BOOL EMIL_BUILD_TESTS)
emil_add_test(protobuf.meta_services_test)

target_sources(protobuf.meta_services_test PRIVATE
    TestServiceDiscoveryEcho.cpp
)

target_link_libraries(protobuf.meta_services_test PUBLIC
    gmock_main
    protobuf.test_doubles
    protobuf.meta_services
)
