add_library(services.util_test_doubles ${EMIL_EXCLUDE_FROM_ALL} INTERFACE)

target_link_libraries(services.util_test_doubles INTERFACE
    gmock
    services.util
)

target_sources(services.util_test_doubles PRIVATE
    ConfigurationStoreMock.hpp
    StoppableMock.hpp
    WritableConfigurationMock.hpp
)
