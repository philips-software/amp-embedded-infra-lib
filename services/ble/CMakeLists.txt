add_library(services.ble ${EMIL_EXCLUDE_FROM_ALL} STATIC)
install(TARGETS services.ble EXPORT emilServicesTargets)

target_link_libraries(services.ble PUBLIC
    hal.interfaces
    infra.util
)

target_sources(services.ble PRIVATE
    BondStorageManager.cpp
    BondStorageManager.hpp
    Gap.hpp
    Gatt.cpp
    Gatt.hpp
    GattCharacteristicImpl.cpp
    GattCharacteristicImpl.hpp
    PersistingBondStorage.cpp
    PersistingBondStorage.hpp
    Uuids.hpp
)

emil_clangformat_target(services.ble)

if (EMIL_BUILD_TESTS)
    add_subdirectory(test)
endif()

if (BUILD_TESTING)
    add_subdirectory(test_doubles)
endif()
