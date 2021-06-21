ccola_component(cucumber static_library)

	ccola_inheritable_include_directories(../..)

    ccola_sources(
        CucumberContext.hpp
        CucumberContext.cpp
        CucumberWireProtocolServer.cpp
        CucumberWireProtocolServer.hpp
        CucumberRequestHandlers.hpp
        CucumberStep.hpp
        CucumberStep.cpp
		CucumberStepMacro.hpp
        CucumberStepStorage.hpp
        CucumberStepStorage.cpp
        CucumberWireProtocolController.cpp
        CucumberWireProtocolController.hpp
        CucumberWireProtocolParser.cpp
        CucumberWireProtocolParser.hpp
        CucumberWireProtocolFormatter.cpp
        CucumberWireProtocolFormatter.hpp
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
		reference_product_mcu.echo_controllers
    )

ccola_end_component()

ccola_subdirectories(
    test
)
