if (TARGET_ARCH_X86)
    add_executable(upgrade.security_key_generator)
    add_executable(emil::upgrade.security_key_generator ALIAS upgrade.security_key_generator)
    install(TARGETS upgrade.security_key_generator EXPORT emilUpgradeHostTargets)

    target_link_libraries(upgrade.security_key_generator PUBLIC
        arg_parser.args
        crypto.mbedtls
        hal.generic
        upgrade.pack_keys
        upgrade.pack_builder
    )

    target_sources(upgrade.security_key_generator PRIVATE
        Main.cpp
        MaterialGenerator.cpp
        MaterialGenerator.hpp
    )
endif()
