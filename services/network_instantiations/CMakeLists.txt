add_library(services.network_instantiations ${EMIL_EXCLUDE_FROM_ALL} STATIC)
target_sources(services.network_instantiations PRIVATE NetworkAdapter.cpp NetworkAdapter.hpp)

if (EMIL_BUILD_WIN)
    target_link_libraries(services.network_instantiations PUBLIC services.network_win)
    target_compile_definitions(services.network_instantiations PUBLIC EMIL_NETWORK_WIN)
endif()

if (EMIL_BUILD_UNIX OR EMIL_BUILD_DARWIN)
    target_link_libraries(services.network_instantiations PUBLIC services.network_bsd)
    target_compile_definitions(services.network_instantiations PUBLIC EMIL_NETWORK_BSD)
endif()
