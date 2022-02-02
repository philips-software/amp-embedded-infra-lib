add_library(services.services_test_doubles EXCLUDE_FROM_ALL STATIC)

target_link_libraries(services.services_test_doubles PUBLIC
    gmock
    services.util
)

target_sources(services.services_test_doubles PRIVATE
    ConfigurationStoreMock.hpp
    StoppableMock.hpp
    WritableConfigurationMock.hpp
)
