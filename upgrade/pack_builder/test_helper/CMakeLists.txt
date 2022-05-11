add_library(upgrade.pack_builder_test_helper ${EMIL_EXCLUDE_FROM_ALL} INTERFACE)

target_sources(upgrade.pack_builder_test_helper PRIVATE
    ZeroFilledString.hpp
)
