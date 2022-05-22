add_library(services.ble ${EMIL_EXCLUDE_FROM_ALL} INTERFACE)
install(TARGETS services.ble EXPORT emilServicesTargets)

target_link_libraries(services.ble INTERFACE
    infra.util
)

target_sources(services.ble PRIVATE
    Gap.hpp
)
