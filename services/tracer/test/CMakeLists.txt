add_executable(services.tracer_test)
gtest_discover_tests(services.tracer_test AUTO)

target_link_libraries(services.tracer_test
    PUBLIC
    services.tracer
    infra.timer_test_helper
    hal.interfaces_test_doubles
    hal.synchronous_test_doubles

    gtest
    gmock
    gmock_main
)

target_sources(services.tracer_test
    PRIVATE
    TestStreamWriterOnSerialCommunication.cpp
    TestStreamWriterOnSynchronousSerialCommunication.cpp
    TestTracer.cpp
    TestTracerAdapterPrintf.cpp
    TestTracerWithDateTime.cpp
)
