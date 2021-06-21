ccola_component(cucumber static_library)

    ccola_inheritable_include_directories(../..)

    ccola_sources(
        CucumberContext.cpp
        CucumberContext.hpp
        CucumberRequestHandlers.hpp
        CucumberStep.cpp
        CucumberStep.hpp
        CucumberStepMacro.hpp
        CucumberStepStorage.cpp
        CucumberStepStorage.hpp
        CucumberWireProtocolController.cpp
        CucumberWireProtocolController.hpp
        CucumberWireProtocolFormatter.cpp
        CucumberWireProtocolFormatter.hpp
        CucumberWireProtocolParser.cpp
        CucumberWireProtocolParser.hpp
        CucumberWireProtocolServer.cpp
        CucumberWireProtocolServer.hpp
        TracingCucumberWireProtocolServer.cpp
        TracingCucumberWireProtocolServer.hpp
    )

    ccola_dependencies(
        infra.event
        infra.stream
        infra.syntax
        infra.timer
        hal.synchronous_interfaces
        services.tracer
    )

ccola_end_component()

ccola_subdirectories(
    test
)
