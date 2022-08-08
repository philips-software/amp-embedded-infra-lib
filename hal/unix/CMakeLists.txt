if (TARGET_BUILD_DARWIN OR TARGET_BUILD_UNIX)
    add_library(hal.unix ${EMIL_EXCLUDE_FROM_ALL} STATIC)
    install(TARGETS hal.unix EXPORT emilHalTargets)

    target_link_libraries(hal.unix PUBLIC
        hal.interfaces
    )

    target_sources(hal.unix PRIVATE
        UartUnix.cpp
        UartUnix.hpp
    )
endif()
