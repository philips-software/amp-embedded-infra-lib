ccola_component(tracer static_library)

    ccola_sources(
        GlobalTracer.cpp
        GlobalTracer.hpp
        StreamWriterOnSerialCommunication.cpp
        StreamWriterOnSerialCommunication.hpp
        StreamWriterOnSynchronousSerialCommunication.cpp
        StreamWriterOnSynchronousSerialCommunication.hpp
        Tracer.cpp
        Tracer.hpp
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
    )

    ccola_dependencies(
        hal.interfaces
        hal.synchronous_interfaces
        infra.stream
    )

ccola_end_component()

ccola_subdirectories(
    test
)
