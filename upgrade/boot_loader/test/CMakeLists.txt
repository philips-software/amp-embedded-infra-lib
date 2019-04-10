ccola_component(boot_loader_test gmock_test)

    ccola_sources(
        TestDecryptorAes.cpp
        TestImageUpgraderEraseSectors.cpp
        TestImageUpgraderInternalFlash.cpp
        TestImageUpgraderSkip.cpp
        TestPackUpgrader.cpp
        TestSecondStageToRamLoader.cpp
    )

    ccola_dependencies(
        boot_loader
        boot_loader_test_doubles
        infra.stream
        hal.synchronous_test_doubles
    )

ccola_end_component()
