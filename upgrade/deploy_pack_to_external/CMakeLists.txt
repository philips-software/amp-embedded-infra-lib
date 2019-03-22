ccola_component(deploy_pack_to_external static_library)

    ccola_sources(
        DeployPackToExternal.cpp
        DeployPackToExternal.hpp
    )

    ccola_dependencies(
        pack
        infra.util
        hal.interfaces
        services.util
    )

ccola_end_component()

ccola_subdirectories(
    test
)
