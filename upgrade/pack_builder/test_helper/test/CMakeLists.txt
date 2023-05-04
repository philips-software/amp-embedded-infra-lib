add_executable(upgrade.pack_builder_test_helper_test)
emil_build_for(upgrade.pack_builder_test_helper_test PREREQUISITE_BOOL EMIL_BUILD_TESTS)
emil_add_test(upgrade.pack_builder_test_helper_test)

target_link_libraries(upgrade.pack_builder_test_helper_test PUBLIC
    gmock_main
    upgrade.pack_builder_test_helper
)

target_sources(upgrade.pack_builder_test_helper_test PRIVATE
    TestZeroFilledString.cpp
)
