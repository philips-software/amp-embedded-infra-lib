add_executable(services.tracer_test)
add_test(NAME services.tracer_test COMMAND services.tracer_test)

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
