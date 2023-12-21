if (NOT EMIL_EXTERNAL_LWIP_TARGET)
    add_subdirectory(lwip_config)
    add_subdirectory(lwip)
endif()

add_subdirectory(lwip_cpp)
