
add_library(upgrade.pack INTERFACE)

target_include_directories(upgrade.pack
    INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(upgrade.pack
    INTERFACE
    infra.util
)

add_library(upgrade.pack_keys EXCLUDE_FROM_ALL STATIC)

protocol_buffer_echo_cpp(upgrade.pack_keys UpgradeKeys.proto)
