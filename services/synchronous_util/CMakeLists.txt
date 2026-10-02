add_library(services.synchronous_util ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(services.synchronous_util PUBLIC
    hal.interfaces
    hal.synchronous_interfaces
    infra.util
)

target_sources(services.synchronous_util PRIVATE
    SynchronousFlashAligner.cpp
    SynchronousFlashDelegate.cpp
    SynchronousFlashQuadSpi.cpp
    SynchronousFlashRegion.cpp
    SynchronousFlashSpi.cpp
    SynchronousSpiMasterWithChipSelect.cpp
    SynchronousFlashAligner.hpp
    SynchronousFlashDelegate.hpp
    SynchronousFlashQuadSpi.hpp
    SynchronousFlashRegion.hpp
    SynchronousFlashSpi.hpp
    SynchronousSpiMasterWithChipSelect.hpp
)

add_library(services.synchronous_sesame ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(services.synchronous_sesame PUBLIC
    hal.synchronous_interfaces
    infra.timer
    protobuf.echo
    services.tracer
    services.util
)

target_sources(services.synchronous_sesame PRIVATE
    SynchronousEchoPolicyDiffieHellman.cpp
    SynchronousEchoPolicyDiffieHellman.hpp
    SynchronousEchoPolicySymmetricKey.cpp
    SynchronousEchoPolicySymmetricKey.hpp
    SynchronousSesameCrypto.hpp
    SynchronousSesameSecured.cpp
    SynchronousSesameSecured.hpp
)

if (EMIL_INCLUDE_MBEDTLS OR NOT EMIL_EXTERNAL_MBEDTLS_TARGET STREQUAL "")
    target_sources(services.synchronous_sesame PRIVATE
        SynchronousEchoInstantiationSecured.cpp
        SynchronousEchoInstantiationSecured.hpp
        SynchronousSesameCryptoMbedTls.cpp
        SynchronousSesameCryptoMbedTls.hpp
        SynchronousSesameInstantiationSecured.cpp
        SynchronousSesameInstantiationSecured.hpp
        SynchronousTracingEchoInstantiationSecured.cpp
        SynchronousTracingEchoInstantiationSecured.hpp
    )
endif()

add_subdirectory(test)
