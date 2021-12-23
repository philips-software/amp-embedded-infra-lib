add_library(services.synchronous_util EXCLUDE_FROM_ALL STATIC)
add_library(emil::services.synchronous_util ALIAS services.synchronous_util)
install(TARGETS services.synchronous_util EXPORT emilServicesTargets)

target_link_libraries(services.synchronous_util PUBLIC
    hal.synchronous_interfaces
)

target_sources(services.synchronous_util PRIVATE
    SynchronousFlashRegion.cpp
    SynchronousFlashQuadSpi.cpp
    SynchronousFlashSpi.cpp
    SynchronousSpiMasterWithChipSelect.cpp
    SynchronousFlashRegion.hpp
    SynchronousFlashQuadSpi.hpp
    SynchronousFlashSpi.hpp
    SynchronousSpiMasterWithChipSelect.hpp
)
