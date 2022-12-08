add_library(examples.network_example ${EMIL_EXCLUDE_FROM_ALL} STATIC)

if (EMIL_BUILD_WIN)
    target_sources(examples.network_example PRIVATE MainWin.cpp)
    target_link_libraries(examples.network_example PUBLIC services.network_win)
endif()

if (EMIL_BUILD_UNIX OR EMIL_BUILD_DARWIN)
    target_sources(examples.network_example PRIVATE MainUnix.cpp)
    target_link_libraries(examples.network_example PUBLIC services.network_bsd)
endif()
