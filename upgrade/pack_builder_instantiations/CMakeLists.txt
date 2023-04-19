if (EMIL_HOST_BUILD)
    add_library(upgrade.pack_builder_instantiations ${EMIL_EXCLUDE_FROM_ALL} STATIC)

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
endif()
