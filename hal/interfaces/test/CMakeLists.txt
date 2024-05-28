add_executable(hal.interfaces_test)
emil_build_for(hal.interfaces_test BOOL EMIL_BUILD_TESTS)
emil_add_test(hal.interfaces_test)

target_link_libraries(hal.interfaces_test PUBLIC
    gmock_main
    hal.interfaces
    hal.interfaces_test_doubles
    infra.event_test_helper
)

target_sources(hal.interfaces_test PRIVATE
    TestCan.cpp
    TestFlash.cpp
    TestFlashHeterogeneous.cpp
    TestFlashHomogeneous.cpp
    TestI2cRegisterAccess.cpp
    TestMacAddress.cpp
    TestQuadSpi.cpp
)
