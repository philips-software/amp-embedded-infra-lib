add_executable(services.synchronous_util_test)
emil_build_for(services.synchronous_util_test BOOL EMIL_BUILD_TESTS)
emil_add_test(services.synchronous_util_test)

target_link_libraries(services.synchronous_util_test PUBLIC
    gmock_main
    hal.synchronous_interfaces_test_doubles
    services.synchronous_util
)

target_sources(services.synchronous_util_test PRIVATE
    TestSynchronousFlashAligner.cpp
    TestSynchronousFlashDelegate.cpp
    TestSynchronousFlashSpi.cpp
)

add_executable(services.synchronous_sesame_test)
emil_build_for(services.synchronous_sesame_test BOOL EMIL_BUILD_TESTS)
emil_add_test(services.synchronous_sesame_test)

if (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang|AppleClang")
    target_compile_options(services.synchronous_sesame_test PRIVATE -Wno-braced-scalar-init)
endif()

target_link_libraries(services.synchronous_sesame_test PUBLIC
    gmock_main
    hal.interfaces_test_doubles
    infra.event_test_helper
    infra.timer_test_helper
    infra.util_test_helper
    protobuf.test_doubles
    services.synchronous_sesame
    services.util_test_doubles
)

target_sources(services.synchronous_sesame_test PRIVATE
    $<$<BOOL:${EMIL_INCLUDE_MBEDTLS}>:TestSynchronousEchoInstantiationSecured.cpp>
    $<$<BOOL:${EMIL_INCLUDE_MBEDTLS}>:TestSynchronousEchoPolicyDiffieHellman.cpp>
    $<$<BOOL:${EMIL_INCLUDE_MBEDTLS}>:TestSynchronousEchoPolicySymmetricKey.cpp>
    $<$<BOOL:${EMIL_INCLUDE_MBEDTLS}>:TestSynchronousSesameCryptoMbedTls.cpp>
    $<$<BOOL:${EMIL_INCLUDE_MBEDTLS}>:TestSynchronousSesameInstantiationSecured.cpp>
    $<$<BOOL:${EMIL_INCLUDE_MBEDTLS}>:TestSynchronousSesameSecured.cpp>
)
