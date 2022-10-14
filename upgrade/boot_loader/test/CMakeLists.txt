add_executable(upgrade.boot_loader_test)
emil_add_test(upgrade.boot_loader_test)

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

emil_clangformat_target(upgrade.boot_loader_test)
