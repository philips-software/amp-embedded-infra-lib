ccola_component(services_test_doubles x86: source other: noncompile)

    ccola_sources(
        ConfigurationStoreMock.hpp
        StoppableMock.hpp
        WritableConfigurationMock.hpp
    )

    ccola_dependencies(
        util
        gmock.gmock
    )

ccola_end_component()
