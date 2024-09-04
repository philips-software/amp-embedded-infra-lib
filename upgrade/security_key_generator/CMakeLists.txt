add_executable(upgrade.security_key_generator)
emil_build_for(upgrade.security_key_generator HOST All PREREQUISITE_BOOL EMIL_STANDALONE)
emil_install(upgrade.security_key_generator EXPORT emilUpgradeTargets DESTINATION bin)

target_link_libraries(upgrade.security_key_generator PUBLIC
    args
    upgrade.material_generator
)

target_sources(upgrade.security_key_generator PRIVATE
    Main.cpp
)

add_subdirectory(material_generator)