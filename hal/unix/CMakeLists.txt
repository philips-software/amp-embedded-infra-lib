if (EMIL_BUILD_UNIX OR EMIL_BUILD_DARWIN)
    add_library(hal.unix ${EMIL_EXCLUDE_FROM_ALL} STATIC)
    install(TARGETS hal.unix EXPORT emilHalTargets)

    target_link_libraries(hal.unix PUBLIC
        hal.interfaces
    )

    target_compile_definitions(hal.unix PUBLIC
        EMIL_HAL_UNIX
    )

    target_sources(hal.unix PRIVATE
        UartUnix.cpp
        UartUnix.hpp
    )
endif()
