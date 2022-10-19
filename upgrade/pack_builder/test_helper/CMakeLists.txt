add_library(upgrade.pack_builder_test_helper ${EMIL_EXCLUDE_FROM_ALL} INTERFACE)

target_sources(upgrade.pack_builder_test_helper PRIVATE
    ZeroFilledString.hpp
)

target_include_directories(upgrade.pack_builder_test_helper INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../..>"
)

emil_clangformat_target(upgrade.pack_builder_test_helper)

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()
