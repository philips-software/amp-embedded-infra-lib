add_library(lwip.lwip_cpp ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(lwip.lwip_cpp PUBLIC
    services.network
    infra.event
    hal.interfaces
)

if (EMIL_EXTERNAL_LWIP_TARGET)
    target_link_libraries(lwip.lwip_cpp PUBLIC
        ${EMIL_EXTERNAL_LWIP_TARGET}
    )
else()
    target_link_libraries(lwip.lwip_cpp PUBLIC
        lwip.lwip
    )
endif()

if (${CMAKE_CXX_COMPILER_ID} STREQUAL AppleClang)
    target_compile_options(lwip.lwip_cpp PRIVATE -Wno-macro-redefined)
endif()

target_sources(lwip.lwip_cpp PRIVATE
    ConnectionLwIp.cpp
    ConnectionLwIp.hpp
    DatagramLwIp.cpp
    DatagramLwIp.hpp
    LightweightIp.cpp
    LightweightIp.hpp
    LightweightIpOverEthernet.cpp
    LightweightIpOverEthernet.hpp
    MulticastLwIp.cpp
    MulticastLwIp.hpp
)
