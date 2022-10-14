add_executable(services.tracer_test)
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
)

emil_clangformat_target(services.tracer_test)
