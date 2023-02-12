add_executable(services.synchronous_util_test)
emil_add_test(services.synchronous_util_test)

target_link_libraries(services.synchronous_util_test PUBLIC
    gmock_main
    hal.synchronous_interfaces_test_doubles
    services.synchronous_util
)

target_sources(services.synchronous_util_test PRIVATE
    TestSynchronousFlashSpi.cpp
)
