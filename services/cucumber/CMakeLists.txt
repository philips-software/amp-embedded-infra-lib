add_library(services.cucumber ${EMIL_EXCLUDE_FROM_ALL} STATIC)
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

emil_clangformat_target(services.cucumber)

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()
