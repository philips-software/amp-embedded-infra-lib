add_executable(upgrade.boot_loader_test)
gtest_discover_tests(upgrade.boot_loader_test AUTO)

target_link_libraries(upgrade.boot_loader_test PUBLIC
    gmock_main
    upgrade.boot_loader
    upgrade.boot_loader_test_doubles
    hal.synchronous_interfaces_test_doubles
)

target_sources(upgrade.boot_loader_test PRIVATE
    TestDecryptorAes.cpp
    TestImageUpgraderEraseSectors.cpp
    TestImageUpgraderFlash.cpp
    TestImageUpgraderSkip.cpp
    TestPackUpgrader.cpp
    TestSecondStageToRamLoader.cpp
    TestVerifierEcDsa.cpp
    TestVerifierHashOnly.cpp
)
