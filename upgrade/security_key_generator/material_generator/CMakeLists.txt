add_library(upgrade.material_generator STATIC EXCLUDE_FROM_ALL)

target_link_libraries(upgrade.material_generator PUBLIC
    hal.generic
    upgrade.pack_keys
    upgrade.pack_builder
)

target_sources(upgrade.material_generator PRIVATE
    MaterialGenerator.cpp
    MaterialGenerator.hpp
)

add_subdirectory(test)