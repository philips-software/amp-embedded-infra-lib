add_executable(services.util_test)
gtest_discover_tests(services.util_test AUTO)

target_link_libraries(services.util_test
    PUBLIC
    services.tracer
    services.util
    hal.interfaces_test_doubles
    infra.timer_test_helper
    infra.util_test_helper

    gtest
    gmock
    gmock_main
)

target_sources(services.util_test
    PRIVATE
    TestConfigurationStore.cpp
    TestCyclicStore.cpp
    TestDebouncedButton.cpp
    TestDebugLed.cpp
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
    TestMessageCommunicationWindowed.cpp
    TestMessageCommunicationCobs.cpp
    TestRepeatingButton.cpp
    TestSignalLed.cpp
    TestSpiMasterWithChipSelect.cpp
    TestSpiMultipleAccess.cpp
    TestTerminal.cpp
    TestTimeWithLocalization.cpp
    TestWritableConfiguration.cpp
)
