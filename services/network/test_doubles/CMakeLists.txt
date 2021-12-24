
add_library(services.network_test_doubles EXCLUDE_FROM_ALL STATIC)

target_link_libraries(services.network_test_doubles
    PUBLIC

    services.network
    gmock
)

target_sources(services.network_test_doubles PRIVATE
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
    HttpClientAuthenticationStub.hpp
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
