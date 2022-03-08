add_library(upgrade.pack INTERFACE)

target_include_directories(upgrade.pack INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(upgrade.pack INTERFACE
    infra.util
)

add_library(upgrade.pack_keys EXCLUDE_FROM_ALL STATIC)

install(TARGETS upgrade.pack_keys upgrade.pack EXPORT emilUpgradeTargets)

protocol_buffer_echo_cpp(upgrade.pack_keys UpgradeKeys.proto)
