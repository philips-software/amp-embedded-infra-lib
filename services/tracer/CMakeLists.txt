add_library(services.tracer ${EMIL_EXCLUDE_FROM_ALL} STATIC)

if (EMIL_ENABLE_TRACING)
    target_compile_definitions(services.tracer PUBLIC EMIL_ENABLE_TRACING)
else()
    target_compile_definitions(services.tracer PUBLIC EMIL_DISABLE_TRACING)
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

add_subdirectory(test)
