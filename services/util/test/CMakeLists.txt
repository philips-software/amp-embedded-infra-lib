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
)

target_sources(services.util_test PRIVATE
    TestAesMbedTls.cpp
    TestConfigurationStore.cpp
    TestCyclicStore.cpp
    TestDebouncedButton.cpp
    TestDebugLed.cpp
    TestEchoOnMessageCommunication.cpp
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
    TestMessageCommunicationSecured.cpp
    TestMessageCommunicationWindowed.cpp
    TestRepeatingButton.cpp
    TestSignalLed.cpp
    TestSpiMasterWithChipSelect.cpp
    TestSpiMultipleAccess.cpp
    TestTerminal.cpp
    TestTimeWithLocalization.cpp
    TestWritableConfiguration.cpp
)
