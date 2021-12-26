add_library(upgrade.deploy_pack_to_external EXCLUDE_FROM_ALL STATIC)
add_library(emil::upgrade.deploy_pack_to_external ALIAS upgrade.deploy_pack_to_external)
install(TARGETS upgrade.deploy_pack_to_external EXPORT emilUpgradeTargets)

target_link_libraries(upgrade.deploy_pack_to_external PUBLIC
    hal.interfaces
    infra.util
    services.util
    upgrade.pack
)

target_sources(upgrade.deploy_pack_to_external PRIVATE
    DeployPackToExternal.cpp
    DeployPackToExternal.hpp
)

if (BUILD_TESTING)
    add_subdirectory(test)
endif()
