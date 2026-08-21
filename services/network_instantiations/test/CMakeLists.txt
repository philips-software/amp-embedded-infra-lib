add_executable(services.network_instantiations_test)
emil_build_for(services.network_instantiations_test BOOL EMIL_BUILD_TESTS PREREQUISITE_BOOL EMIL_STANDALONE)
emil_add_test(services.network_instantiations_test)

target_link_libraries(services.network_instantiations_test PUBLIC
    gmock_main
    services.network_instantiations
)

target_sources(services.network_instantiations_test PRIVATE
    TestSocketAddress.cpp
)
