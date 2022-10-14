add_library(lwip.lwip_cpp ${EMIL_EXCLUDE_FROM_ALL} STATIC)
install(TARGETS lwip.lwip_cpp EXPORT emilLwipTargets)

target_link_libraries(lwip.lwip_cpp PUBLIC
    services.network
    infra.event
    hal.interfaces
)

if (LWIP_ALTERNATIVE_DEPEND)
    target_link_libraries(lwip.lwip_cpp PUBLIC
        ${LWIP_ALTERNATIVE_DEPEND}
    )
else()
    target_link_libraries(lwip.lwip_cpp PUBLIC
        lwip.lwip
    )
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

emil_clangformat_target(lwip.lwip_cpp)
