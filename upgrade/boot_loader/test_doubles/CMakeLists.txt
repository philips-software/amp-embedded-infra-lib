add_library(upgrade.boot_loader_test_doubles STATIC)
emil_build_for(upgrade.boot_loader_test_doubles BOOL BUILD_TESTING)

target_link_libraries(upgrade.boot_loader_test_doubles PRIVATE
    gmock
    upgrade.boot_loader
)

target_sources(upgrade.boot_loader_test_doubles PRIVATE
    MockDecryptor.cpp
    MockDecryptor.hpp
)
