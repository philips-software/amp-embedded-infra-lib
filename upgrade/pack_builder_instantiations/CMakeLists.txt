add_library(upgrade.pack_builder_instantiations STATIC)
emil_build_for(upgrade.pack_builder_instantiations HOST All PREREQUISITE_BOOL EMIL_STANDALONE)

target_link_libraries(upgrade.pack_builder_instantiations PUBLIC
    args
    hal.generic
    upgrade.pack_builder
)

target_sources(upgrade.pack_builder_instantiations PRIVATE
    UpgradePackBuilderApplication.cpp
    UpgradePackBuilderApplication.hpp
    UpgradePackBuilderFacade.cpp
    UpgradePackBuilderFacade.hpp
)
