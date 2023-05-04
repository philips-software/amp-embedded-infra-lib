add_library(services.util_test_doubles INTERFACE)
emil_build_for(services.util_test_doubles PREREQUISITE_BOOL BUILD_TESTING)

target_link_libraries(services.util_test_doubles INTERFACE
    gmock
    services.util
)

target_sources(services.util_test_doubles PRIVATE
    ConfigurationStoreMock.hpp
    StoppableMock.hpp
    WritableConfigurationMock.hpp
)
