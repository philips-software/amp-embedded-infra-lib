add_executable(upgrade.deploy_pack_to_external_test)
gtest_discover_tests(upgrade.deploy_pack_to_external_test AUTO)

target_link_libraries(upgrade.deploy_pack_to_external_test PUBLIC
    gmock_main
    hal.interfaces_test_doubles
    infra.timer_test_helper
    upgrade.deploy_pack_to_external
)

target_sources(upgrade.deploy_pack_to_external_test PRIVATE
    TestDeployPackToExternal.cpp
)
