if (NOT LWIP_ALTERNATIVE_DEPEND)
    add_subdirectory(lwip_config)
    add_subdirectory(lwip-2.1.1)
endif()

add_subdirectory(lwip_cpp)
