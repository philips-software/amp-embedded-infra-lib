if (NOT EMIL_EXTERNAL_LWIP_TARGET)
    add_subdirectory(lwip_config)
    add_subdirectory(lwip-2.1.1)
endif()

add_subdirectory(lwip_cpp)
