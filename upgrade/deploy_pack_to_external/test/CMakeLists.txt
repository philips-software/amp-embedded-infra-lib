ccola_component(deploy_pack_to_external_test gmock_test)

    ccola_sources(
        TestDeployPackToExternal.cpp
    )

    ccola_dependencies(
        deploy_pack_to_external
        hal.test_doubles
        infra.timer_test_helper
    )

ccola_end_component()
