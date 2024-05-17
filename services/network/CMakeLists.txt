add_library(services.network ${EMIL_EXCLUDE_FROM_ALL} STATIC)

protocol_buffer_echo_cpp(services.network Network.proto)
protocol_buffer_csharp(services.network Network.proto)
protocol_buffer_java(services.network Network.proto)

target_link_libraries(services.network PUBLIC
    infra.syntax
    infra.timer
    hal.synchronous_interfaces
    protobuf.echo
    services.tracer
    services.util
)

target_sources(services.network PRIVATE
    Address.cpp
    Address.hpp
    BonjourServer.cpp
    BonjourServer.hpp
    $<$<BOOL:${EMIL_HOST_BUILD}>:CertificateConvertor.cpp>
    $<$<BOOL:${EMIL_HOST_BUILD}>:CertificateConvertor.hpp>
    Connection.cpp
    Connection.hpp
    ConnectionFactoryWithNameResolver.cpp
    ConnectionFactoryWithNameResolver.hpp
    ConnectionStatus.hpp
    Datagram.hpp
    Dns.cpp
    Dns.hpp
    DnsResolver.cpp
    DnsResolver.hpp
    DnsServer.cpp
    DnsServer.hpp
    EchoOnConnection.cpp
    EchoOnConnection.hpp
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
    TracingEchoOnConnection.cpp
    TracingEchoOnConnection.hpp
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
    WiFiNetwork.cpp
    WiFiNetwork.hpp
)

if (EMIL_HOST_BUILD)
    target_sources(services.network PRIVATE
        CertificateConvertor.cpp
        CertificateConvertor.hpp
    )
endif()

if (EMIL_INCLUDE_MBEDTLS OR EMIL_EXTERNAL_MBEDTLS_TARGET)
    if (EMIL_EXTERNAL_MBEDTLS_TARGET)
        target_link_libraries(services.network PUBLIC
            ${EMIL_EXTERNAL_MBEDTLS_TARGET}
        )
    else()
        target_link_libraries(services.network PUBLIC
            mbedcrypto
        )
    endif()

    target_sources(services.network PRIVATE
        CertificatesMbedTls.cpp
        CertificatesMbedTls.hpp
        ConnectionMbedTls.cpp
        ConnectionMbedTls.hpp
        HttpPageWebSocket.cpp
        HttpPageWebSocket.hpp
        NameResolverCache.cpp
        NameResolverCache.hpp
        TracingConnectionMbedTls.cpp
        TracingConnectionMbedTls.hpp
        WebSocketClientConnectionObserver.cpp
        WebSocketClientConnectionObserver.hpp
        WebSocketServerConnectionObserver.cpp
        WebSocketServerConnectionObserver.hpp
    )
endif()

add_subdirectory(test)
add_subdirectory(test_doubles)
