add_executable(upgrade.boot_loader_test)
gtest_discover_tests(upgrade.boot_loader_test AUTO)

target_link_libraries(upgrade.boot_loader_test
    PUBLIC
    upgrade.boot_loader
    upgrade.boot_loader_test_doubles
    infra.stream
    hal.synchronous_interfaces_test_doubles

    gtest
    gmock
    gmock_main
)

target_sources(upgrade.boot_loader_test
    PRIVATE
    TestDecryptorAes.cpp
    TestImageUpgraderEraseSectors.cpp
    TestImageUpgraderFlash.cpp
    TestImageUpgraderSkip.cpp
    TestPackUpgrader.cpp
    TestSecondStageToRamLoader.cpp
)
