if (EMIL_HOST_BUILD)
    add_executable(upgrade.security_key_generator)
    install(TARGETS upgrade.security_key_generator EXPORT emilUpgradeTargets)

    target_link_libraries(upgrade.security_key_generator PUBLIC
        args
        mbedcrypto
        hal.generic
        upgrade.pack_keys
        upgrade.pack_builder
    )

    target_sources(upgrade.security_key_generator PRIVATE
        Main.cpp
        MaterialGenerator.cpp
        MaterialGenerator.hpp
    )

    emil_clangformat_target(upgrade.security_key_generator)
endif()
