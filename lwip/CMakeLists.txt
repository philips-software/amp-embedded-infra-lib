ccola_package(lwip)

    if(NOT LWIP_ALTERNATIVE_DEPEND)
        ccola_subdirectories(
            lwip_config
            lwip-2.1.1
        )
    endif()

    ccola_subdirectories(
        lwip_cpp
    )

ccola_end_package()
