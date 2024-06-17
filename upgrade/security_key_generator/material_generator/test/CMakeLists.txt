add_executable(upgrade.material_generator_test)
emil_build_for(upgrade.material_generator_test BOOL EMIL_BUILD_TESTS)
emil_add_test(upgrade.material_generator_test)

target_link_libraries(upgrade.material_generator_test PUBLIC
    gmock_main
    upgrade.material_generator
    hal.interfaces_test_doubles
    infra.syntax
)

target_sources(upgrade.material_generator_test PRIVATE
    TestMaterialGenerator.cpp
)
