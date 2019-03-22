ccola_component(tracer_test gmock_test)

    ccola_sources(
        TestStreamWriterOnSerialCommunication.cpp
        TestStreamWriterOnSynchronousSerialCommunication.cpp
        TestTracer.cpp
        TestTracerAdapterPrintf.cpp
        TestTracerWithDateTime.cpp
    )

    ccola_dependencies(
        tracer
        infra.timer_test_helper
        hal.test_doubles
        hal.synchronous_test_doubles
    )

ccola_end_component()
