ccola_component(util_test gmock_test)

    ccola_sources(
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
        TestMessageCommunication.cpp
        TestMessageCommunicationCobs.cpp
        TestRepeatingButton.cpp
        TestSignalLed.cpp
        TestSpiMasterWithChipSelect.cpp
        TestSpiMultipleAccess.cpp
        TestTerminal.cpp
        TestTimeWithLocalization.cpp
        TestWritableConfiguration.cpp
    )

    ccola_dependencies(
        tracer
        util
        hal.test_doubles
        infra.timer_test_helper
        infra.util_test_helper
    )

ccola_end_component()
