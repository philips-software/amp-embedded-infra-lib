add_executable(services.network_test)
emil_add_test(services.network_test)

target_link_libraries(services.network_test PUBLIC
    gmock_main
    services.network
    services.network_test_doubles
    infra.timer_test_helper
    infra.util_test_helper
    hal.generic
    hal.interfaces_test_doubles
    hal.synchronous_interfaces_test_doubles
    services.util
)

target_sources(services.network_test PRIVATE
    TestAddress.cpp
    TestBonjourServer.cpp
    TestCertificateConvertor.cpp
    TestCertificatesMbedTls.cpp
    TestConnectionMbedTls.cpp
    TestDnsResolver.cpp
    TestDnsServer.cpp
    TestExclusiveConnection.cpp
    TestExclusiveStartingConnection.cpp
    TestHttpClient.cpp
    TestHttpClientAuthentication.cpp
    TestHttpClientBasic.cpp
    TestHttpClientJson.cpp
    TestHttpServer.cpp
    TestLlmnrResponder.cpp
    TestMdnsClient.cpp
    TestMqttClient.cpp
    TestNameResolverCache.cpp
    TestSerialServer.cpp
    TestSingleConnectionListener.cpp
    TestSntpClient.cpp
    TestSsdpDeviceDiscovery.cpp
    TestWebSocketClient.cpp
    TestWebSocketClientConnectionObserver.cpp
    TestWebSocketIntegration.cpp
    TestWebSocketServerConnectionObserver.cpp
    TestWiFiNetwork.cpp
)
