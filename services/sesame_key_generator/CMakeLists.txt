if (EMIL_HOST_BUILD AND EMIL_INCLUDE_MBEDTLS)
    add_executable(services.sesame_key_generator ${EMIL_EXCLUDE_FROM_ALL})
    emil_install(services.sesame_key_generator DESTINATION bin)

    target_link_libraries(services.sesame_key_generator PUBLIC
        args
        services.util
    )

    target_sources(services.sesame_key_generator PRIVATE
        Main.cpp
    )
endif()
