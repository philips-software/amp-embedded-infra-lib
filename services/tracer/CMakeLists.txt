add_library(services.tracer ${EMIL_EXCLUDE_FROM_ALL} STATIC)
install(TARGETS services.tracer EXPORT emilServicesTargets)

target_link_libraries(services.tracer PUBLIC
    hal.interfaces
    hal.synchronous_interfaces
    infra.stream
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
)

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()
