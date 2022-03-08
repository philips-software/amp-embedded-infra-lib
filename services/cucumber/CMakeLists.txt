add_library(services.cucumber EXCLUDE_FROM_ALL STATIC)
install(TARGETS services.cucumber EXPORT emilServicesTargets)

target_link_libraries(services.cucumber PUBLIC
    infra.syntax
    infra.timer
    hal.synchronous_interfaces
    services.tracer
)

target_sources(services.cucumber PRIVATE
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

if (BUILD_TESTING)
    add_subdirectory(test)
endif()
