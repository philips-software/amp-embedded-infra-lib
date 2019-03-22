ccola_component(network_test gmock_test)

    ccola_sources(
        TestCertificatesMbedTls.cpp
        TestConnectionMbedTls.cpp
        TestConnectionSerial.cpp
        TestHttpClient.cpp
        TestHttpClientBasic.cpp
        TestHttpServer.cpp
        TestLlmnrResponder.cpp
        TestMqttClient.cpp
        TestSntpClient.cpp
        TestWebSocketClient.cpp
    )

    ccola_dependencies(
        network
        network_test_doubles
        infra.timer_test_helper
        infra.util_test_helper
        hal.test_doubles
        services.util
    win:
        hal.windows
    )

ccola_end_component()
