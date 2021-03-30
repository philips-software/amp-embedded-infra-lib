ccola_component(cucumber static_library)

    ccola_sources(
        CucumberWireProtocolServer.cpp
        CucumberWireProtocolServer.hpp
        CucumberStep.hpp
        CucumberStep.cpp
		CucumberStepMacro.hpp
        CucumberStepStorage.hpp
        CucumberStepStorage.cpp
        CucumberWireProtocolParser.hpp
        CucumberWireProtocolParser.cpp
        TracingCucumberWireProtocolServer.cpp
        TracingCucumberWireProtocolServer.hpp
    )

    ccola_dependencies(
        infra.event
        infra.stream
        infra.syntax
        infra.timer
        crypto.mbed_tls
        hal.synchronous_interfaces
        services.tracer
    )

ccola_end_component()

ccola_subdirectories(
    test
)
