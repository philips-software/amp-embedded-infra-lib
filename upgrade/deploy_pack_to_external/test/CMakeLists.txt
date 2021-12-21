add_executable(upgrade.deploy_pack_to_external_test)
gtest_discover_tests(upgrade.deploy_pack_to_external_test AUTO)

target_link_libraries(upgrade.deploy_pack_to_external_test
    PUBLIC
    upgrade.deploy_pack_to_external
    hal.interfaces_test_doubles
    infra.timer_test_helper

    gtest
    gmock
    gmock_main
)

target_sources(upgrade.deploy_pack_to_external_test
    PRIVATE
    TestDeployPackToExternal.cpp
)
