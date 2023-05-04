add_library(services.network ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(services.network PUBLIC
    infra.syntax
    infra.timer
    hal.synchronous_interfaces
    services.tracer
    services.util
    mbedtls
)

target_sources(services.network PRIVATE
    Address.cpp
    Address.hpp
    BonjourServer.cpp
    BonjourServer.hpp
    $<$<BOOL:${EMIL_HOST_BUILD}>:CertificateConvertor.cpp>
    $<$<BOOL:${EMIL_HOST_BUILD}>:CertificateConvertor.hpp>
    CertificatesMbedTls.cpp
    CertificatesMbedTls.hpp
    Connection.cpp
    Connection.hpp
    ConnectionFactoryWithNameResolver.cpp
    ConnectionFactoryWithNameResolver.hpp
    ConnectionMbedTls.cpp
    ConnectionMbedTls.hpp
    ConnectionStatus.hpp
    Datagram.hpp
    Dns.cpp
    Dns.hpp
    DnsResolver.cpp
    DnsResolver.hpp
    DnsServer.cpp
    DnsServer.hpp
    ExclusiveConnection.cpp
    ExclusiveConnection.hpp
    ExclusiveStartingConnection.cpp
    ExclusiveStartingConnection.hpp
    Http.cpp
    Http.hpp
    HttpClient.hpp
    HttpClientAuthentication.cpp
    HttpClientAuthentication.hpp
    HttpClientBasic.cpp
    HttpClientBasic.hpp
    HttpClientCachedConnection.cpp
    HttpClientCachedConnection.hpp
    HttpClientImpl.cpp
    HttpClientImpl.hpp
    HttpClientJson.cpp
    HttpClientJson.hpp
    HttpErrors.cpp
    HttpErrors.hpp
    HttpPageWebSocket.cpp
    HttpPageWebSocket.hpp
    HttpRequestParser.cpp
    HttpRequestParser.hpp
    HttpServer.cpp
    HttpServer.hpp
    LlmnrResponder.cpp
    LlmnrResponder.hpp
    MdnsClient.cpp
    MdnsClient.hpp
    Mqtt.hpp
    MqttClientImpl.cpp
    MqttClientImpl.hpp
    MqttMultipleAccess.cpp
    MqttMultipleAccess.hpp
    Multicast.hpp
    NameResolver.hpp
    NameResolverCache.cpp
    NameResolverCache.hpp
    Network.hpp
    SerialServer.cpp
    SerialServer.hpp
    SingleConnectionListener.cpp
    SingleConnectionListener.hpp
    SntpClient.cpp
    SntpClient.hpp
    SsdpDeviceDiscovery.cpp
    SsdpDeviceDiscovery.hpp
    StreamEcho.cpp
    StreamEcho.hpp
    TracingConnectionFactoryWithNameResolver.cpp
    TracingConnectionFactoryWithNameResolver.hpp
    TracingConnectionMbedTls.cpp
    TracingConnectionMbedTls.hpp
    TracingHttpClientCachedConnection.cpp
    TracingHttpClientCachedConnection.hpp
    TracingHttpClientImpl.cpp
    TracingHttpClientImpl.hpp
    TracingMqttClientImpl.cpp
    TracingMqttClientImpl.hpp
    TracingStatusHttpClientImpl.cpp
    TracingStatusHttpClientImpl.hpp
    WebSocket.cpp
    WebSocket.hpp
    WebSocketClientConnectionObserver.cpp
    WebSocketClientConnectionObserver.hpp
    WebSocketServerConnectionObserver.cpp
    WebSocketServerConnectionObserver.hpp
    WiFiNetwork.cpp
    WiFiNetwork.hpp
)

if (EMIL_HOST_BUILD)
    target_sources(services.network PRIVATE
        CertificateConvertor.cpp
        CertificateConvertor.hpp
    )
endif()

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (BUILD_TESTING)
    add_subdirectory(test_doubles)
endif()
