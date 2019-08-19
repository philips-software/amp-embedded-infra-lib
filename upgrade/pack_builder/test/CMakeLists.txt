ccola_component(pack_builder_test gmock_test)

    ccola_sources(
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

    ccola_dependencies(
        pack_builder
        infra.timer_test_helper
        hal.synchronous_test_doubles
        hal.test_doubles
        infra.syntax
    )

ccola_end_component()
