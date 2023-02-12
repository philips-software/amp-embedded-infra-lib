add_executable(upgrade.pack_builder_test)
emil_add_test(upgrade.pack_builder_test)

target_link_libraries(upgrade.pack_builder_test PUBLIC
    gmock_main
    hal.synchronous_interfaces_test_doubles
    hal.interfaces_test_doubles
    infra.syntax
    infra.timer_test_helper
    upgrade.pack_builder
    upgrade.pack_builder_test_helper
)

target_sources(upgrade.pack_builder_test PRIVATE
    TestBinaryObject.cpp
    TestConfigParser.cpp
    TestImageAuthenticatorHmac.cpp
    TestImageEncryptorAes.cpp
    TestImageSignerEcDsa.cpp
    TestImageSignerHashOnly.cpp
    TestInputBinary.cpp
    TestInputCommand.cpp
    TestInputHex.cpp
    TestSparseVector.cpp
    TestSupportedTargets.cpp
    TestUpgradePackBuilder.cpp
    TestUpgradePackInputFactory.cpp
)
