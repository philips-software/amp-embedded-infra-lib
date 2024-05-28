add_executable(services.util_test)
emil_build_for(services.util_test BOOL EMIL_BUILD_TESTS)
emil_add_test(services.util_test)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|AppleClang")
    target_compile_options(services.util_test PRIVATE -Wno-braced-scalar-init)
endif()

target_link_libraries(services.util_test PUBLIC
    gmock_main
    protobuf.test_doubles
    services.tracer
    services.util
    hal.interfaces_test_doubles
    infra.timer_test_helper
    infra.util_test_helper
    infra.event_test_helper
)

target_sources(services.util_test PRIVATE
    $<$<BOOL:${EMIL_INCLUDE_MBEDTLS}>:TestAesMbedTls.cpp>
    $<$<BOOL:${EMIL_INCLUDE_MBEDTLS}>:TestConfigurationStore.cpp>
    TestCyclicStore.cpp
    TestDebouncedButton.cpp
    TestDebugLed.cpp
    $<$<BOOL:${EMIL_INCLUDE_MBEDTLS}>:TestEchoOnMessageCommunication.cpp>
    $<$<BOOL:${EMIL_INCLUDE_MBEDTLS}>:TestEchoOnMessageCommunicationSymmetricKey.cpp>
    TestFlashAlign.cpp
    TestFlashMultipleAccess.cpp
    TestFlashQuadSpiCypressFll.cpp
    TestFlashQuadSpiMicronN25q.cpp
    TestFlashQuadSpiSingleSpeed.cpp
    TestFlashRegion.cpp
    TestFlashSpi.cpp
    TestI2cMultipleAccess.cpp
    TestInverseLogicPin.cpp
    TestLowPowerSerialCommunication.cpp
    TestLowPowerSpiMaster.cpp
    TestMessageCommunicationCobs.cpp
    $<$<BOOL:${EMIL_INCLUDE_MBEDTLS}>:TestMessageCommunicationSecured.cpp>
    TestMessageCommunicationWindowed.cpp
    TestRepeatingButton.cpp
    TestSerialCommunicationLoopback.cpp
    TestSignalLed.cpp
    TestSpiMasterWithChipSelect.cpp
    TestSpiMultipleAccess.cpp
    TestTerminal.cpp
    TestTimeWithLocalization.cpp
    $<$<BOOL:${EMIL_INCLUDE_MBEDTLS}>:TestWritableConfiguration.cpp>
)
