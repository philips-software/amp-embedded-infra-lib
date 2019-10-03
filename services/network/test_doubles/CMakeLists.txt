ccola_component(network_test_doubles x86: static_library other: noncompile)

    ccola_sources(
        AddressMock.hpp
        ConnectionFactoryWithNameResolverStub.cpp
        ConnectionFactoryWithNameResolverStub.hpp
        ConnectionLoopBack.cpp
        ConnectionLoopBack.hpp
        ConnectionMock.cpp
        ConnectionMock.hpp
        ConnectionStub.cpp
        ConnectionStub.hpp
        DatagramMock.hpp
        HttpClientMock.hpp
        HttpClientMock.cpp
        HttpRequestParserStub.hpp
        HttpServerMock.hpp
        MqttMock.hpp
        MulticastMock.hpp
        NameResolverMock.hpp
        SntpMock.hpp
        WiFiNetworkMock.hpp
    )

    ccola_dependencies(
        network
        gmock.gmock
    )

ccola_end_component()
