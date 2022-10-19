add_library(upgrade.boot_loader_test_doubles ${EMIL_EXCLUDE_FROM_ALL} STATIC)

target_link_libraries(upgrade.boot_loader_test_doubles PRIVATE
    gmock
    upgrade.boot_loader
)

target_sources(upgrade.boot_loader_test_doubles PRIVATE
    MockDecryptor.cpp
    MockDecryptor.hpp
)

emil_clangformat_target(upgrade.boot_loader_test_doubles)
