add_library(upgrade.deploy_pack_to_external ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(upgrade.deploy_pack_to_external PUBLIC
    hal.interfaces
    services.util
    upgrade.pack
)

target_sources(upgrade.deploy_pack_to_external PRIVATE
    DeployPackToExternal.cpp
    DeployPackToExternal.hpp
)

add_subdirectory(test)
