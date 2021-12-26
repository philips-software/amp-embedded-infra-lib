add_library(upgrade.pack INTERFACE)
add_library(emil::upgrade.pack ALIAS upgrade.pack)
install(TARGETS upgrade.pack EXPORT emilUpgradeTargets)

target_include_directories(upgrade.pack INTERFACE
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(upgrade.pack INTERFACE
    infra.util
)

add_library(upgrade.pack_keys EXCLUDE_FROM_ALL STATIC)
add_library(emil::pack_keys ALIAS upgrade.pack_keys)
install(TARGETS upgrade.pack_keys EXPORT emilUpgradeTargets)

protocol_buffer_echo_cpp(upgrade.pack_keys UpgradeKeys.proto)
