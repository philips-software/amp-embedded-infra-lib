add_library(upgrade.deploy_pack_to_external ${EMIL_EXCLUDE_FROM_ALL} STATIC)
install(TARGETS upgrade.deploy_pack_to_external EXPORT emilUpgradeTargets)

target_link_libraries(upgrade.deploy_pack_to_external PUBLIC
    hal.interfaces
    services.util
    upgrade.pack
)

target_sources(upgrade.deploy_pack_to_external PRIVATE
    DeployPackToExternal.cpp
    DeployPackToExternal.hpp
)

emil_clangformat_target(upgrade.deploy_pack_to_external)

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()
