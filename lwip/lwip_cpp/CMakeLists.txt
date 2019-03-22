ccola_component(lwip_cpp static_library)

    ccola_sources(
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

    ccola_dependencies(
        services.network
        infra.event
        hal.interfaces
    )

    if(LWIP_ALTERNATIVE_DEPEND)
        ccola_dependencies(
            ${LWIP_ALTERNATIVE_DEPEND}
        )
    else()
        ccola_dependencies(
            lwip
        )
    endif()

ccola_end_component()
