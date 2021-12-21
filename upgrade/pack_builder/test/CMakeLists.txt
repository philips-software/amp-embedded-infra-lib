add_executable(upgrade.pack_builder_test)
gtest_discover_tests(upgrade.pack_builder_test AUTO)

target_link_libraries(upgrade.pack_builder_test
    PUBLIC
    hal.synchronous_interfaces_test_doubles
    hal.interfaces_test_doubles
    infra.syntax
    infra.timer_test_helper
    upgrade.pack_builder

    gtest
    gmock
    gmock_main
)

target_sources(upgrade.pack_builder_test
    PRIVATE
    ImageSecurityNone.cpp
    ImageSecurityNone.hpp
    TestBinaryObject.cpp
    TestConfigParser.cpp
    TestImageAuthenticatorHmac.cpp
    TestImageEncryptorAes.cpp
    TestImageSecurityXteaHmac.cpp
    TestImageSignerEcDsa.cpp
    TestImageSignerRsa.cpp
    TestInputBinary.cpp
    TestInputHex.cpp
    TestSparseVector.cpp
    TestUpgradePackBuilder.cpp
)
