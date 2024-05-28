add_executable(upgrade.security_key_generator)
emil_build_for(upgrade.security_key_generator HOST All PREREQUISITE_BOOL EMIL_STANDALONE)
emil_install(upgrade.security_key_generator EXPORT emilUpgradeTargets DESTINATION bin)

target_link_libraries(upgrade.security_key_generator PUBLIC
    args
    hal.generic
    upgrade.pack_keys
    upgrade.pack_builder
)

target_sources(upgrade.security_key_generator PRIVATE
    Main.cpp
    MaterialGenerator.cpp
    MaterialGenerator.hpp
)
