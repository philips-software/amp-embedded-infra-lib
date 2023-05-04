add_library(upgrade.pack_builder_test_helper INTERFACE)
emil_build_for(upgrade.pack_builder_test_helper PREREQUISITE_BOOL BUILD_TESTING)

target_sources(upgrade.pack_builder_test_helper PRIVATE
    ZeroFilledString.hpp
)

target_include_directories(upgrade.pack_builder_test_helper INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../../..>"
)

add_subdirectory(test)
