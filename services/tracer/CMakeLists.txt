add_library(services.tracer ${EMIL_EXCLUDE_FROM_ALL} STATIC)

if (EMIL_ENABLE_TRACING)
    target_compile_definitions(services.tracer PUBLIC EMIL_ENABLE_TRACING=1)
else()
    target_compile_definitions(services.tracer PUBLIC EMIL_DISABLE_TRACING=1)
endif()

target_link_libraries(services.tracer PUBLIC
    hal.interfaces
    hal.synchronous_interfaces
    infra.stream
    infra.timer
    services.util
)

target_sources(services.tracer PRIVATE
    GlobalTracer.cpp
    GlobalTracer.hpp
    StreamWriterOnSerialCommunication.cpp
    StreamWriterOnSerialCommunication.hpp
    StreamWriterOnSynchronousSerialCommunication.cpp
    StreamWriterOnSynchronousSerialCommunication.hpp
    Tracer.cpp
    Tracer.hpp
    TracerOnIoOutputInfrastructure.cpp
    TracerOnIoOutputInfrastructure.hpp
    TracerWithDateTime.cpp
    TracerWithDateTime.hpp
    TracerWithTime.cpp
    TracerWithTime.hpp
    TracerAdapterPrintf.cpp
    TracerAdapterPrintf.hpp
    TracingFlash.cpp
    TracingFlash.hpp
    TracingInputStream.cpp
    TracingInputStream.hpp
    TracingOutputStream.cpp
    TracingOutputStream.hpp
    TracingReset.cpp
    TracingReset.hpp
)

if (EMIL_INCLUDE_SEGGER_RTT)
    target_link_libraries(services.tracer PUBLIC
        segger.rtt
    )

    target_sources(services.tracer PRIVATE
        StreamWriterOnSeggerRtt.cpp
        StreamWriterOnSeggerRtt.hpp
        TracerOnSeggerRttInfrastructure.cpp
        TracerOnSeggerRttInfrastructure.hpp
    )
endif()

add_subdirectory(test)
