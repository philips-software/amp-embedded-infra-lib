add_executable(services.tracer_test)
emil_build_for(services.tracer_test PREREQUISITE_BOOL EMIL_BUILD_TESTS)
emil_add_test(services.tracer_test)

target_link_libraries(services.tracer_test PUBLIC
    gmock_main
    services.tracer
    infra.timer_test_helper
    hal.interfaces_test_doubles
    hal.synchronous_interfaces_test_doubles
)

target_sources(services.tracer_test PRIVATE
    TestStreamWriterOnSerialCommunication.cpp
    TestStreamWriterOnSynchronousSerialCommunication.cpp
    TestTracer.cpp
    TestTracerAdapterPrintf.cpp
    TestTracerWithDateTime.cpp
    TestTracerWithTime.cpp
    TestTracingReset.cpp
)
