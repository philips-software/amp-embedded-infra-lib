add_executable(upgrade.deploy_pack_to_external_test)
emil_build_for(upgrade.deploy_pack_to_external_test BOOL EMIL_BUILD_TESTS)
emil_add_test(upgrade.deploy_pack_to_external_test)

target_link_libraries(upgrade.deploy_pack_to_external_test PUBLIC
    gmock_main
    hal.interfaces_test_doubles
    infra.timer_test_helper
    upgrade.deploy_pack_to_external
)

target_sources(upgrade.deploy_pack_to_external_test PRIVATE
    TestDeployPackToExternal.cpp
)
