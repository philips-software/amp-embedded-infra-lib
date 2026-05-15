add_executable(services.tracer_test)
emil_build_for(services.tracer_test BOOL EMIL_BUILD_TESTS)
emil_add_test(services.tracer_test)

target_link_libraries(services.tracer_test PUBLIC
    gmock_main
    services.tracer
    infra.timer_test_helper
    hal.interfaces_test_doubles
    hal.synchronous_interfaces_test_doubles
)

target_sources(services.tracer_test PRIVATE
    TestLogAndAbortTracer.cpp
    TestStreamWriterOnSerialCommunication.cpp
    TestStreamWriterOnFlushableSerialCommunication.cpp
    TestStreamWriterOnSynchronousSerialCommunication.cpp
    TestTracer.cpp
    TestTracerAdapterPrintf.cpp
    TestTracerWithDateTime.cpp
    TestTracerWithTime.cpp
    TestTracingReset.cpp
)

add_library(services.tracer_disabled ${EMIL_EXCLUDE_FROM_ALL} STATIC)
target_compile_definitions(services.tracer_disabled PUBLIC EMIL_DISABLE_TRACING=1)
target_link_libraries(services.tracer_disabled PUBLIC
    infra.stream
)
target_sources(services.tracer_disabled PRIVATE
    ../Tracer.cpp
    ../Tracer.hpp
)

add_executable(services.tracer_disabled_test)
emil_build_for(services.tracer_disabled_test BOOL EMIL_BUILD_TESTS)
emil_add_test(services.tracer_disabled_test)

target_link_libraries(services.tracer_disabled_test PUBLIC
    gmock_main
    services.tracer_disabled
)

target_sources(services.tracer_disabled_test PRIVATE
    TestTracerDisabled.cpp
)
