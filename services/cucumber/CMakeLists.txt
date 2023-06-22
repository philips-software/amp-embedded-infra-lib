add_library(services.cucumber ${EMIL_EXCLUDE_FROM_ALL} STATIC)

option(EMIL_CUCUMBER_USE_GMOCK "Use the GoogleTest/GoogleMock adapter in the Cucumber implementation; this allows the use of GoogleMock checks and assertions" On)

target_link_libraries(services.cucumber PUBLIC
    infra.syntax
    infra.timer
    hal.synchronous_interfaces
    services.network
)

target_sources(services.cucumber PRIVATE
    CucumberContext.hpp
    CucumberRequestHandler.cpp
    CucumberRequestHandler.hpp
    CucumberStep.cpp
    CucumberStep.hpp
    CucumberStepMacro.hpp
    CucumberStepStorage.cpp
    CucumberStepStorage.hpp
    CucumberWireProtocolFormatter.cpp
    CucumberWireProtocolFormatter.hpp
    CucumberWireProtocolParser.cpp
    CucumberWireProtocolParser.hpp
    CucumberWireProtocolServer.cpp
    CucumberWireProtocolServer.hpp
    TracingCucumberWireProtocolServer.cpp
    TracingCucumberWireProtocolServer.hpp
)

if (EMIL_CUCUMBER_USE_GMOCK)
    emil_fetch_googletest()

    target_link_libraries(services.cucumber PUBLIC GTest::gmock)
    target_compile_definitions(services.cucumber PRIVATE EMIL_CUCUMBER_USE_GMOCK)
endif()

#add_subdirectory(test)
